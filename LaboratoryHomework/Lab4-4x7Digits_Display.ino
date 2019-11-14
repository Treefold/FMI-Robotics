enum: uint8_t
{
  pinA = 2, 
  pinB,
  pinC,
  pinD,
  pinE,
  pinF,
  pinG,
  pinDP,
  pinD1,
  pinD2,
  pinD3,
  pinD4,

  pinVRx = A0,
  pinVRy = A1,
  pinSW  = A2
};
const uint8_t number[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x64, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
uint8_t currentNr[4] = {0, 0, 0, 0}, currDig = 0;
void printNumber (uint8_t nr) {
  for (int pin = pinA; pin < pinDP; ++pin) {
    digitalWrite (pin, number[nr] & (1 << (pin - pinA)));
  }
  digitalWrite (pinDP, nr % 2);
}
void updateNr()
{
  for (int i = 0; i < 4; ++i) {digitalWrite(pinD1 + i, LOW); printNumber(currentNr[i]);digitalWrite(pinD1 + i, HIGH);}
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pinSW, INPUT_PULLUP);
  for (int pin = pinA;  pin <= pinDP; ++pin) {pinMode (pin, OUTPUT);}
  for (int pin = pinD1; pin <= pinD4; ++pin) {pinMode (pin, OUTPUT); digitalWrite(pin, HIGH);}
  digitalWrite (pinD1 + currDig, LOW);
  printNumber (0);
}
void loop() {
  //updateNr();
  static int readX, readY, updatedX = 0, updatedY = 0;
  if (digitalRead(pinSW))
  {
    updatedX = 0;
    readY = analogRead(pinVRy);  
    if (readY < 200 && updatedY != 1) {if(currentNr[currDig] == 9){currentNr[currDig] = 0;}else{++currentNr[currDig];}printNumber(currentNr[currDig]);updatedY = 1;}
    if (readY > 800 && updatedY != 2) {if(currentNr[currDig] == 0){currentNr[currDig] = 9;}else{--currentNr[currDig];}printNumber(currentNr[currDig]);updatedY = 2;}
    if (400 <= readY && readY <= 600) {                                                                                                               updatedY = 0;}
  }
  else
  {
    updatedY = 0;
    readX = analogRead(pinVRx);
    if (readX > 700 && updatedX != 1) {digitalWrite(pinD1 + currDig, HIGH); if (currDig == 3){currDig = 0;}else{++currDig;}digitalWrite(pinD1 + currDig, LOW);printNumber(currentNr[currDig]);updatedX = 1;}
    if (readX < 300 && updatedX != 2) {digitalWrite(pinD1 + currDig, HIGH); if (currDig == 0){currDig = 3;}else{--currDig;}digitalWrite(pinD1 + currDig, LOW);printNumber(currentNr[currDig]);updatedX = 2;}
    if (400 <= readX && readX <= 600) {                                                                                                                                                       updatedX = 0;}
  }
}
