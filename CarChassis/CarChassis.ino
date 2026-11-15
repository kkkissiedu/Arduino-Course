/*
  PROJECT: L298N SERIAL MOTOR CONTROLLER
  --------------------------------------
  Control two DC motors independently or together.

  COMMAND FORMAT:
  MOTOR/SPEED/DURATION/DIRECTION

  EXAMPLES:
  LEFT/200/3000/FORWARD
  RIGHT/150/2000/BACKWARD
  BOTH/255/1000/FORWARD
  LEFT/0/0/STOP

  SPEED:     0 - 255
  DURATION:  milliseconds
  DIRECTION: FORWARD, BACKWARD, STOP

  Set Serial Monitor line ending to:
  Newline
*/

// ---------------- PIN CONFIGURATION ----------------

// Left motor (Motor A)
#define ENA 5
#define IN1 8
#define IN2 9

// Right motor (Motor B)
#define ENB 6
#define IN3 10
#define IN4 11

// ----------------------------------------------------

String command = "";

struct Motor {
  int enablePin;
  int input1;
  int input2;
};

Motor leftMotor  = {ENA, IN1, IN2};
Motor rightMotor = {ENB, IN3, IN4};


// Stop a motor
void stopMotor(Motor motor) {
  analogWrite(motor.enablePin, 0);

  digitalWrite(motor.input1, LOW);
  digitalWrite(motor.input2, LOW);
}


// Control motor direction and speed
void controlMotor(Motor motor, int speed, String direction) {

  speed = constrain(speed, 0, 255);

  if (direction == "FORWARD") {

    digitalWrite(motor.input1, HIGH);
    digitalWrite(motor.input2, LOW);

  }

  else if (direction == "BACKWARD") {

    digitalWrite(motor.input1, LOW);
    digitalWrite(motor.input2, HIGH);

  }

  else {

    stopMotor(motor);
    return;

  }

  analogWrite(motor.enablePin, speed);
}


// Execute a command
void executeCommand(String cmd) {

  cmd.trim();
  cmd.toUpperCase();

  // Split command into four parts
  int firstSlash  = cmd.indexOf('/');
  int secondSlash = cmd.indexOf('/', firstSlash + 1);
  int thirdSlash  = cmd.indexOf('/', secondSlash + 1);

  if (firstSlash < 0 || secondSlash < 0 || thirdSlash < 0) {
    Serial.println("ERROR: Invalid command format!");
    Serial.println("Use MOTOR/SPEED/DURATION/DIRECTION");
    return;
  }

  String motorName = cmd.substring(0, firstSlash);

  int speed = cmd.substring(
    firstSlash + 1, secondSlash
  ).toInt();

  unsigned long duration = cmd.substring(
    secondSlash + 1, thirdSlash
  ).toInt();

  String direction = cmd.substring(thirdSlash + 1);

  // Validate speed
  if (speed < 0 || speed > 255) {
    Serial.println("ERROR: Speed must be between 0 and 255.");
    return;
  }

  if (direction != "FORWARD" &&
      direction != "BACKWARD" &&
      direction != "STOP") {

    Serial.println("ERROR: Invalid direction.");
    return;
  }

  // Execute command
  if (motorName == "LEFT") {

    controlMotor(leftMotor, speed, direction);

  }

  else if (motorName == "RIGHT") {

    controlMotor(rightMotor, speed, direction);

  }

  else if (motorName == "BOTH") {

    controlMotor(leftMotor, speed, direction);
    controlMotor(rightMotor, speed, direction);

  }

  else {
    Serial.println("ERROR: Motor must be LEFT, RIGHT or BOTH.");
    return;
  }

  Serial.print("Executing: ");
  Serial.println(cmd);

  // Keep motor running for specified duration
  if (direction != "STOP") {
    delay(duration);
  }

  // Automatically stop after duration
  if (motorName == "LEFT") {
    stopMotor(leftMotor);
  }

  else if (motorName == "RIGHT") {
    stopMotor(rightMotor);
  }

  else if (motorName == "BOTH") {
    stopMotor(leftMotor);
    stopMotor(rightMotor);
  }

  Serial.println("Motor stopped.");
}


// Arduino setup
void setup() {

  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotor(leftMotor);
  stopMotor(rightMotor);

  Serial.println("================================");
  Serial.println("  L298N MOTOR CONTROL SYSTEM");
  Serial.println("================================");

  Serial.println("LEFT/200/3000/FORWARD");
  Serial.println("RIGHT/150/2000/BACKWARD");
  Serial.println("BOTH/255/1000/FORWARD");

  Serial.println("Ready for commands!");
}


// Main loop
void loop() {

  while (Serial.available()) {

    char incoming = Serial.read();

    if (incoming == '\n') {

      executeCommand(command);

      command = "";

    }

    else if (incoming != '\r') {

      command += incoming;

    }

  }

}