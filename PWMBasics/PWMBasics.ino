int pwmPin = 3;
int  bright = 126;

void setup() {
  pinMode(pwmPin, OUTPUT)
  // put your setup code here, to run once:
}

void loop() {
  analogWrite(pwmPin, bright);
  // put your main code here, to run repeatedly:
}
