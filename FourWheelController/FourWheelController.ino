/*
  =====================================================
       FOUR-WHEEL ROBOT CONTROL SYSTEM
       2 x L298N MOTOR DRIVERS
  =====================================================

  COMMAND FORMAT:
  MOTOR/SPEED/DURATION/DIRECTION

  MOTOR:
    FL     = Front Left
    FR     = Front Right
    RL     = Rear Left
    RR     = Rear Right
    LEFT   = Both left wheels
    RIGHT  = Both right wheels
    FRONT  = Both front wheels
    REAR   = Both rear wheels
    ALL    = All four wheels
    BOTH   = Alias for ALL

  SPEED:
    0 - 255

  DURATION:
    Time in milliseconds.
    0 = Run continuously until another command stops it.

  DIRECTION:
    FORWARD
    BACKWARD
    STOP

  EXAMPLES:
    ALL/200/3000/FORWARD
    LEFT/150/2000/BACKWARD
    FL/255/1000/FORWARD
    RIGHT/0/0/STOP

  Serial Monitor:
    Baud rate: 9600
    Line ending: Newline
*/

// =====================================================
// 1. MOTOR PIN CONFIGURATION
// =====================================================

// FRONT LEFT
#define FL_EN 3
#define FL_IN1 4
#define FL_IN2 7

// FRONT RIGHT
#define FR_EN 5
#define FR_IN1 8
#define FR_IN2 12

// REAR LEFT
#define RL_EN 6
#define RL_IN1 2
#define RL_IN2 A0

// REAR RIGHT
#define RR_EN 9
#define RR_IN1 A1
#define RR_IN2 A2


// =====================================================
// 2. MOTOR STRUCTURE
// =====================================================

struct Motor {

  int enablePin;
  int in1;
  int in2;

  bool invert;   // true flips direction so FORWARD is forward on this wheel

  unsigned long stopTime;
  bool running;
  bool timed;

};


// Create four motor objects (FR, RL and RR run backwards when wired as-is, so they are inverted)

Motor FL = {FL_EN, FL_IN1, FL_IN2, false, 0, false, false};
Motor FR = {FR_EN, FR_IN1, FR_IN2, true,  0, false, false};
Motor RL = {RL_EN, RL_IN1, RL_IN2, true,  0, false, false};
Motor RR = {RR_EN, RR_IN1, RR_IN2, true,  0, false, false};


// Serial command buffer
String command = "";


// =====================================================
// 3. STOP MOTOR
// =====================================================

void stopMotor(Motor &motor) {

  analogWrite(motor.enablePin, 0);

  digitalWrite(motor.in1, LOW);
  digitalWrite(motor.in2, LOW);

  motor.running = false;
  motor.timed = false;

}


// =====================================================
// 4. CONTROL MOTOR
// =====================================================

void controlMotor(
  Motor &motor,
  int speed,
  unsigned long duration,
  String direction
) {

  speed = constrain(speed, 0, 255);

  // Stop if STOP was requested or speed is zero
  if (direction == "STOP" || speed == 0) {

    stopMotor(motor);
    return;

  }

  // Set motor direction (flipped for inverted wheels)
  bool forward = (direction == "FORWARD");

  if (motor.invert) {
    forward = !forward;
  }

  digitalWrite(motor.in1, forward ? HIGH : LOW);
  digitalWrite(motor.in2, forward ? LOW : HIGH);

  // Apply speed using PWM
  analogWrite(motor.enablePin, speed);

  motor.running = true;

  // Duration of zero means continuous operation
  if (duration == 0) {

    motor.timed = false;

  }

  else {

    motor.timed = true;
    motor.stopTime = millis() + duration;

  }

}


// =====================================================
// 5. UPDATE MOTOR TIMERS
// =====================================================

void updateMotor(Motor &motor) {

  if (motor.running && motor.timed) {

    // Check whether this motor's time has expired
    if ((long)(millis() - motor.stopTime) >= 0) {

      stopMotor(motor);

      Serial.println("A motor has completed its duration.");

    }

  }

}


// =====================================================
// 6. SELECT AND CONTROL WHEELS
// =====================================================

void executeCommand(String cmd) {

  cmd.trim();
  cmd.toUpperCase();

  int firstSlash = cmd.indexOf('/');
  int secondSlash = cmd.indexOf('/', firstSlash + 1);
  int thirdSlash = cmd.indexOf('/', secondSlash + 1);

  if (firstSlash < 0 ||
      secondSlash < 0 ||
      thirdSlash < 0) {

    Serial.println("ERROR: Invalid command format.");
    return;

  }

  String motorName = cmd.substring(0, firstSlash);

  String speedText = cmd.substring(
    firstSlash + 1, secondSlash
  );

  String durationText = cmd.substring(
    secondSlash + 1, thirdSlash
  );

  String direction = cmd.substring(thirdSlash + 1);

  // Validate numeric fields before conversion
  if (speedText.length() == 0 ||
      durationText.length() == 0) {

    Serial.println("ERROR: Missing speed or duration.");
    return;

  }

  for (unsigned int i = 0; i < speedText.length(); i++) {

    if (!isDigit(speedText[i])) {
      Serial.println("ERROR: Speed must be a number.");
      return;
    }

  }

  for (unsigned int i = 0; i < durationText.length(); i++) {

    if (!isDigit(durationText[i])) {
      Serial.println("ERROR: Duration must be a number.");
      return;
    }

  }

  long speedValue = speedText.toInt();
  unsigned long duration = strtoul(
    durationText.c_str(), NULL, 10
  );

  if (speedValue < 0 || speedValue > 255) {

    Serial.println("ERROR: Speed must be 0-255.");
    return;

  }

  if (direction != "FORWARD" &&
      direction != "BACKWARD" &&
      direction != "STOP") {

    Serial.println("ERROR: Invalid direction.");
    return;

  }


  // Decide which motors should receive the command

  bool useFL = false;
  bool useFR = false;
  bool useRL = false;
  bool useRR = false;


  if (motorName == "FL") {
    useFL = true;
  }

  else if (motorName == "FR") {
    useFR = true;
  }

  else if (motorName == "RL") {
    useRL = true;
  }

  else if (motorName == "RR") {
    useRR = true;
  }

  else if (motorName == "LEFT") {
    useFL = true;
    useRL = true;
  }

  else if (motorName == "RIGHT") {
    useFR = true;
    useRR = true;
  }

  else if (motorName == "FRONT") {
    useFL = true;
    useFR = true;
  }

  else if (motorName == "REAR") {
    useRL = true;
    useRR = true;
  }

  else if (motorName == "ALL" || motorName == "BOTH") {
    useFL = true;
    useFR = true;
    useRL = true;
    useRR = true;
  }

  else {

    Serial.println("ERROR: Unknown motor selection.");
    return;

  }


  // Apply commands to selected motors

  if (useFL) {
    controlMotor(FL, speedValue, duration, direction);
  }

  if (useFR) {
    controlMotor(FR, speedValue, duration, direction);
  }

  if (useRL) {
    controlMotor(RL, speedValue, duration, direction);
  }

  if (useRR) {
    controlMotor(RR, speedValue, duration, direction);
  }


  // Confirmation message

  Serial.print("COMMAND: ");
  Serial.println(cmd);

  Serial.println("Command successfully processed.");

}


// =====================================================
// 7. ARDUINO SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  // Configure all motor pins

  pinMode(FL_EN, OUTPUT);
  pinMode(FL_IN1, OUTPUT);
  pinMode(FL_IN2, OUTPUT);

  pinMode(FR_EN, OUTPUT);
  pinMode(FR_IN1, OUTPUT);
  pinMode(FR_IN2, OUTPUT);

  pinMode(RL_EN, OUTPUT);
  pinMode(RL_IN1, OUTPUT);
  pinMode(RL_IN2, OUTPUT);

  pinMode(RR_EN, OUTPUT);
  pinMode(RR_IN1, OUTPUT);
  pinMode(RR_IN2, OUTPUT);

  // Ensure all wheels are stopped at startup

  stopMotor(FL);
  stopMotor(FR);
  stopMotor(RL);
  stopMotor(RR);

  Serial.println();
  Serial.println("==================================");
  Serial.println("    FOUR WHEEL ROBOT CONTROLLER");
  Serial.println("==================================");

  Serial.println("FL/200/2000/FORWARD");
  Serial.println("FR/200/2000/FORWARD");
  Serial.println("LEFT/150/3000/FORWARD");
  Serial.println("RIGHT/150/3000/BACKWARD");
  Serial.println("ALL/200/5000/FORWARD");

  Serial.println("----------------------------------");
  Serial.println("SYSTEM READY!");
  Serial.println("----------------------------------");

}


// =====================================================
// 8. MAIN LOOP
// =====================================================

void loop() {

  // Continuously monitor all four motor timers

  updateMotor(FL);
  updateMotor(FR);
  updateMotor(RL);
  updateMotor(RR);


  // Read incoming serial commands

  while (Serial.available()) {

    char incoming = Serial.read();

    if (incoming == '\n') {

      executeCommand(command);

      command = "";

    }

    else if (incoming != '\r') {

      if (command.length() < 100) {
        command += incoming;
      }

      else {
        command = "";
        Serial.println("ERROR: Command too long.");
      }

    }

  }

}
