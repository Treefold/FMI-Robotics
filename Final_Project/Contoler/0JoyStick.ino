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


uint8_t joystickState () {
  // state = 0x000byyxx where:
  // b = button state: 0 - not pressed, 1 - pressed
  // yy = 00 - nothing, 01 - up,   10 - down
  // xx = 00 - nothing, 01 - left, 10 - right
  uint16_t state = 0,
           x     = analogRead   (A1),
           y     = analogRead   (A0),
           btn   = !digitalRead (3);
  if (x > 767) { // left
    state |= (1);
  }
  else { 
    if (x < 256) { // right
      state |= (2);
    }
    // else not modified
  }
  
  if (y > 767) { // up
    state |= (1) << 2;
  }
  else { 
    if (y < 256) { // down
      state |= (2) << 2;
    }
    // else not modified
  }
      
  state |= (btn) << 4; 

  return state;
}
