const int pinI0 = A0;
const int pinI1 = A1;
const int pinI2 = A2;
const int pinO0 = 9;
const int pinO1 = 10;
const int pinO2 = 11;

void setup() {
  // put your setup code here, to run once:
  pinMode (pinI0, INPUT);
  pinMode (pinI1, INPUT);
  pinMode (pinI2, INPUT);
  pinMode (pinO0, OUTPUT);
  pinMode (pinO1, OUTPUT);
  pinMode (pinO2, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println (map(analogRead(pinI0), 0, 1023, 0, 255));
  analogWrite (pinO0, map(analogRead(pinI0), 0, 1023, 0, 255));
  Serial.println (map(analogRead(pinI1), 0, 1023, 0, 255));
  analogWrite (pinO1, map(analogRead(pinI1), 0, 1023, 0, 255));
  Serial.println (map(analogRead(pinI2), 0, 1023, 0, 255));
  analogWrite (pinO2, map(analogRead(pinI2), 0, 1023, 0, 255));
}
