void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(500);

  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(100);

  digitalWrite(11, HIGH);
  delay(100);

  digitalWrite(12, HIGH);
  digitalWrite(11, LOW);
  delay(100);

  digitalWrite(13, HIGH);
  digitalWrite(12, LOW);
  delay(100);

  digitalWrite(13, LOW);
  delay(100); 

  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(100);

  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(100);

  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(100);

  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(100);

  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(100);




  
  // put your main code here, to run repeatedly:
}
