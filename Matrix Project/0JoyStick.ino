// JoyStick JS
const uint8_t  Js_vrxPin     = A0,
               Js_vryPin     = A1,
               Js_btnPin     = 13;
const bool     Js_btnPressed = 0;
const uint16_t Js_LeftPoint  = 1023 / 4,
               Js_RightPoint = 1023 * 3 / 4,
               Js_UpPoint    = 1023 / 4,
               Js_DownPoint  = 1023  * 3 / 4,
               Js_NoPoint    = 1023 / 2;
uint16_t       Js_vrxValue   = Js_NoPoint,
               Js_vryValue   = Js_NoPoint;
bool           Js_btnValue   = !Js_btnPressed;

void Js_Init() {
  pinMode (Js_btnPin, INPUT_PULLUP);
}

void Js_ReadX () {
  Js_vrxValue = analogRead (Js_vrxPin);
}
void Js_ReadY () {
  Js_vryValue = analogRead (Js_vryPin);
}

bool Js_btnIsPressed() {
  return (digitalRead (Js_btnPin) == Js_btnPressed
          && digitalRead (Js_btnPin) == Js_btnPressed
          && digitalRead (Js_btnPin) == Js_btnPressed);
}
