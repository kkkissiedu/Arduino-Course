int redPin = 3; 
int bright = 0;
int delayT = 100;

void setup() {
  pinMode(redPin, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  analogWrite(redPin, bright);
  delay(delayT);
  bright = bright + 10;
  // put your main code here, to run repeatedly:
}
