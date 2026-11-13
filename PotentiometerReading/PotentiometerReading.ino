int potPin = A2;
int potVal;
int br = 9600;
int waitT = 100;
float volts;

int bright;
int redPin = 3;

void setup() {
  pinMode(potPin, INPUT);
  Serial.begin(br);

  pinMode(redPin, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  potVal = analogRead(potPin);
  volts = (5.0/1023.0) * potVal;

  bright = (255.0/ 1023.0) * potVal;
  analogWrite(redPin, bright);

  Serial.print("potVal: ");
  Serial.print(potVal); 
  Serial.print("   Voltage: ");
  Serial.print(volts);
  Serial.print("   brightness ");
  Serial.println(bright);

  delay(waitT);
  // put your main code here, to run repeatedly:
}
