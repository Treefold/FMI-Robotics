// JoyStick JS *DO NOT RENAME FILE*
const uint8_t  js_vrxPin     = A0,
               js_vryPin     = A1,
               js_btnPin     = 3;
const bool     js_btnPressed = 1;
const uint16_t js_LeftPoint  = 1023 / 4,
               js_RightPoint = 1023 * 3 / 4,
               js_UpPoint    = 1023 / 4,
               js_DownPoint  = 1023  * 3 / 4,
               js_NoPoint    = 1023 / 2;
uint16_t       js_vrxValue   = js_NoPoint,
               js_vryValue   = js_NoPoint;
bool           js_btnValue   = !js_btnPressed;

void Js_Init() {
  pinMode (js_btnPin, INPUT_PULLUP);
}

void Js_ReadX () {
  js_vrxValue = analogRead (js_vrxPin);
}

void Js_ReadY () {
  js_vryValue = analogRead (js_vryPin);
}

bool Js_btnIsPressed() {
  return (digitalRead (js_btnPin) == js_btnPressed
          && digitalRead (js_btnPin) == js_btnPressed
          && digitalRead (js_btnPin) == js_btnPressed);
}
