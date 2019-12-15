// Game states and meniu states can be change anywhere

enum GameStates : uint8_t { // GS
  GS_Meniu,
  GS_InGame,
  GS_Pause,
  GS_EndGame,
  GS_Hs,
  GS_HsSaveName
} gameState = GS_Meniu;// GS_InGame; // GS_Meniu;

enum MeniuState : uint8_t {
  MS_FirstState, // link to the previous of the last state
  MS_Start,
  MS_Lvl,
  MS_Hs,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Start;

// joystick JS
const uint8_t  Js_vrxPin     = A0,
               Js_vryPin     = A1,
               Js_btnPin     = 13;
const bool     Js_btnPressed = 0;
const uint16_t Js_LeftPoint  = 1023 / 4,
               Js_RightPoint = 1023 * 3 / 4,
               Js_UpPoint    = 1023 * 3 / 4,
               Js_DownPoint  = 1023 / 4,
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

bool Js_btnIsPressed() {
  return (digitalRead (Js_btnPin) == Js_btnPressed
          && digitalRead (Js_btnPin) == Js_btnPressed
          && digitalRead (Js_btnPin) == Js_btnPressed);
}
