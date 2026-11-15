/*
  SAVED-COMMAND REPEATER (four wheels, 2 x L298N)

  The last valid serial command (or comma-separated chain of steps) is stored
  in EEPROM. On power-up or reset the car replays it: step 1, step 2, ... last
  step, pause, then again, forever, until a new command arrives over serial.
  The new command replaces the saved one.

  COMMAND FORMAT (9600 baud, Newline):
    STEP[,STEP,STEP...]            up to MAX_STEPS steps
    STEP = MOTOR/SPEED/DURATION/DIRECTION

    MOTOR     FL FR RL RR LEFT RIGHT FRONT REAR ALL BOTH
    SPEED     0-255
    DURATION  step time in ms (0 = use DEFAULT_RUN_MS)
    DIRECTION FORWARD BACKWARD STOP
              LEFT  / RIGHT = spin on the spot (all four wheels, MOTOR ignored)

  EXAMPLES:
    ALL/200/3000/FORWARD
    ALL/200/2000/FORWARD,ALL/200/700/LEFT                     forward, turn left
    ALL/200/2000/FORWARD,ALL/200/700/LEFT,ALL/200/2000/FORWARD,ALL/200/700/LEFT,
      (x4 on one line)                                          square
    ALL/200/2000/FORWARD,ALL/0/1000/STOP,ALL/200/2000/BACKWARD  STOP step = wait
    ALL/0/0/STOP                                                saves "idle"

  Opening the serial monitor resets the Uno, which replays the saved chain.
*/

#include <EEPROM.h>

// =====================================================
// 1. CONFIG
// =====================================================

const unsigned long DEFAULT_RUN_MS = 3000;   // used when DURATION is 0
const unsigned long STEP_GAP_MS    = 300;    // stopped time between steps
const unsigned long PAUSE_MS       = 1500;   // stopped time between repeats

const int  EE_MAGIC_ADDR = 0;
const int  EE_LEN_ADDR   = 1;
const int  EE_DATA_ADDR  = 2;
const int  MAX_CMD       = 250;
const int  MAX_STEPS     = 8;
const byte EE_MAGIC      = 0xA5;

// Pins (same as four-wheel-controller)
#define FL_EN  3
#define FL_IN1 4
#define FL_IN2 7

#define FR_EN  5
#define FR_IN1 8
#define FR_IN2 12

#define RL_EN  6
#define RL_IN1 2
#define RL_IN2 A0

#define RR_EN  9
#define RR_IN1 A1
#define RR_IN2 A2


// =====================================================
// 2. MOTORS
// =====================================================

struct Motor {
  int  enablePin;
  int  in1;
  int  in2;
  bool invert;   // true flips direction so FORWARD is forward on this wheel
  bool leftSide;
};

// Index = bit position in the motor mask: FL=0, FR=1, RL=2, RR=3
Motor motors[4] = {
  {FL_EN, FL_IN1, FL_IN2, false, true},
  {FR_EN, FR_IN1, FR_IN2, true,  false},
  {RL_EN, RL_IN1, RL_IN2, true,  true},
  {RR_EN, RR_IN1, RR_IN2, true,  false},
};

enum Direction { DIR_STOP, DIR_FORWARD, DIR_BACKWARD, DIR_LEFT, DIR_RIGHT };

struct Command {
  byte          mask;
  int           speed;
  unsigned long duration;
  Direction     dir;
};


void stopMotor(const Motor &m) {
  analogWrite(m.enablePin, 0);
  digitalWrite(m.in1, LOW);
  digitalWrite(m.in2, LOW);
}


void stopAll() {
  for (int i = 0; i < 4; i++) stopMotor(motors[i]);
}


void driveMotor(const Motor &m, int speed, bool forward) {
  if (m.invert) forward = !forward;

  digitalWrite(m.in1, forward ? HIGH : LOW);
  digitalWrite(m.in2, forward ? LOW : HIGH);
  analogWrite(m.enablePin, speed);
}


bool isMoving(const Command &c) {
  return c.dir != DIR_STOP && c.speed > 0;
}


void applyCommand(const Command &c) {
  if (!isMoving(c)) {
    for (int i = 0; i < 4; i++) {
      if (c.mask & (1 << i)) stopMotor(motors[i]);
    }
    return;
  }

  for (int i = 0; i < 4; i++) {
    bool forward;

    if (c.dir == DIR_LEFT || c.dir == DIR_RIGHT) {
      // spin: left wheels back / right wheels forward turns left
      forward = (c.dir == DIR_RIGHT) ? motors[i].leftSide : !motors[i].leftSide;
    }
    else {
      if (!(c.mask & (1 << i))) continue;
      forward = (c.dir == DIR_FORWARD);
    }

    driveMotor(motors[i], c.speed, forward);
  }
}


// =====================================================
// 3. PARSER
// =====================================================

bool isAllDigits(const String &s) {
  if (s.length() == 0) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    if (!isDigit(s[i])) return false;
  }
  return true;
}


bool parseStep(String cmd, Command &out, const char *&err) {
  int s1 = cmd.indexOf('/');
  int s2 = cmd.indexOf('/', s1 + 1);
  int s3 = cmd.indexOf('/', s2 + 1);

  if (s1 < 0 || s2 < 0 || s3 < 0) { err = "Invalid step format."; return false; }

  String name   = cmd.substring(0, s1);
  String speedS = cmd.substring(s1 + 1, s2);
  String durS   = cmd.substring(s2 + 1, s3);
  String dirS   = cmd.substring(s3 + 1);

  if (!isAllDigits(speedS)) { err = "Speed must be a number."; return false; }
  if (!isAllDigits(durS))   { err = "Duration must be a number."; return false; }

  long speed = speedS.toInt();
  if (speed < 0 || speed > 255) { err = "Speed must be 0-255."; return false; }

  if      (dirS == "FORWARD")  out.dir = DIR_FORWARD;
  else if (dirS == "BACKWARD") out.dir = DIR_BACKWARD;
  else if (dirS == "STOP")     out.dir = DIR_STOP;
  else if (dirS == "LEFT")     out.dir = DIR_LEFT;
  else if (dirS == "RIGHT")    out.dir = DIR_RIGHT;
  else { err = "Invalid direction."; return false; }

  if      (name == "FL")    out.mask = 0b0001;
  else if (name == "FR")    out.mask = 0b0010;
  else if (name == "RL")    out.mask = 0b0100;
  else if (name == "RR")    out.mask = 0b1000;
  else if (name == "LEFT")  out.mask = 0b0101;
  else if (name == "RIGHT") out.mask = 0b1010;
  else if (name == "FRONT") out.mask = 0b0011;
  else if (name == "REAR")  out.mask = 0b1100;
  else if (name == "ALL" || name == "BOTH") out.mask = 0b1111;
  else { err = "Unknown motor selection."; return false; }

  out.speed    = (int)speed;
  out.duration = strtoul(durS.c_str(), NULL, 10);
  return true;
}


// Strip spaces + upper-case, so saved text is canonical
String normalised(String s) {
  s.replace(" ", "");
  s.replace("\t", "");
  s.toUpperCase();
  return s;
}


bool parseChain(const String &raw, Command *out, int &count, const char *&err) {
  String s = normalised(raw);
  count = 0;

  if (s.length() == 0) { err = "Empty command."; return false; }

  int start = 0;
  while (start <= (int)s.length()) {
    int comma = s.indexOf(',', start);
    if (comma < 0) comma = s.length();

    if (count >= MAX_STEPS) { err = "Too many steps."; return false; }
    if (!parseStep(s.substring(start, comma), out[count], err)) return false;

    count++;
    start = comma + 1;
  }
  return true;
}


// =====================================================
// 4. EEPROM (last command)
// =====================================================

bool loadSaved(String &cmd) {
  if (EEPROM.read(EE_MAGIC_ADDR) != EE_MAGIC) return false;

  int len = EEPROM.read(EE_LEN_ADDR);
  if (len < 1 || len > MAX_CMD) return false;

  cmd = "";
  for (int i = 0; i < len; i++) cmd += (char)EEPROM.read(EE_DATA_ADDR + i);
  return true;
}


void saveCommand(const String &cmd) {
  String old;
  if (loadSaved(old) && old == cmd) return;   // skip identical write, saves EEPROM wear

  EEPROM.update(EE_MAGIC_ADDR, EE_MAGIC);
  EEPROM.update(EE_LEN_ADDR, cmd.length());
  for (unsigned int i = 0; i < cmd.length(); i++) {
    EEPROM.update(EE_DATA_ADDR + i, cmd[i]);
  }
}


// =====================================================
// 5. REPEAT ENGINE
// =====================================================

Command       steps[MAX_STEPS];
int           stepCount = 0;
int           stepIdx   = 0;
bool          repeating = false;
bool          running   = false;
unsigned long phaseEnd  = 0;


unsigned long stepMs(const Command &c) {
  return c.duration ? c.duration : DEFAULT_RUN_MS;
}


void startStep() {
  applyCommand(steps[stepIdx]);
  running  = true;
  phaseEnd = millis() + stepMs(steps[stepIdx]);
}


void activate() {
  stopAll();

  repeating = false;
  for (int i = 0; i < stepCount; i++) {
    if (isMoving(steps[i])) repeating = true;
  }

  stepIdx = 0;
  running = false;

  if (repeating) startStep();
}


void updateRepeat() {
  if (!repeating) return;

  if ((long)(millis() - phaseEnd) < 0) return;

  if (running) {
    stopAll();
    running = false;
    stepIdx++;

    unsigned long gap = STEP_GAP_MS;
    if (stepIdx >= stepCount) {
      stepIdx = 0;
      gap     = PAUSE_MS;
    }
    phaseEnd = millis() + gap;
  }
  else {
    startStep();
  }
}


// =====================================================
// 6. SERIAL
// =====================================================

String line = "";


void handleLine(const String &raw) {
  Command     parsed[MAX_STEPS];
  int         n   = 0;
  const char *err = "";

  if (!parseChain(raw, parsed, n, err)) {
    Serial.print("ERROR: ");
    Serial.println(err);
    return;
  }

  String cmd = normalised(raw);
  if (cmd.length() > MAX_CMD) {
    Serial.println("ERROR: Command too long to save.");
    return;
  }

  saveCommand(cmd);

  stepCount = n;
  for (int i = 0; i < n; i++) steps[i] = parsed[i];
  activate();

  Serial.print("SAVED + REPEATING (");
  Serial.print(n);
  Serial.print(" steps): ");
  Serial.println(cmd);
}


void readSerial() {
  while (Serial.available()) {
    char ch = Serial.read();

    if (ch == '\n') {
      handleLine(line);
      line = "";
    }
    else if (ch != '\r') {
      if (line.length() < MAX_CMD) {
        line += ch;
      }
      else {
        line = "";
        Serial.println("ERROR: Command too long.");
      }
    }
  }
}


// =====================================================
// 7. ARDUINO ENTRY POINTS
// =====================================================

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(motors[i].enablePin, OUTPUT);
    pinMode(motors[i].in1, OUTPUT);
    pinMode(motors[i].in2, OUTPUT);
  }
  stopAll();

  Serial.begin(9600);
  Serial.println();
  Serial.println("SAVED-COMMAND REPEATER");

  String      saved;
  const char *err = "";

  if (loadSaved(saved) && parseChain(saved, steps, stepCount, err)) {
    Serial.print("BOOT: replaying saved command (");
    Serial.print(stepCount);
    Serial.print(" steps): ");
    Serial.println(saved);
    activate();
  }
  else {
    Serial.println("BOOT: no saved command, idle. Send STEP[,STEP...] e.g. ALL/200/3000/FORWARD");
  }
}


void loop() {
  updateRepeat();
  readSerial();
}
