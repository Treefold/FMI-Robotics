//  *DO NOT RENAME FILE*
//  the files names matter due to the compilling order

void setup() {
  Serial.begin (9600);
  //pinMode (PAUSE_PIN,   INPUT_PULLUP);
  //pinMode (hs_resetPin, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(PAUSE_PIN),   Pause,    LOW);
  //attachInterrupt(digitalPinToInterrupt(hs_resetPin), HS_Reset, FALLING);
  Js_Init();
  //Lcd_Init();
  Matrix_Init();
}

void loop() {
  Js_ReadX();Js_ReadY();
  Serial.println (js_vrxValue);
  Serial.println (js_vryValue);
  Serial.println (Js_btnIsPressed());
  switch (gameState) {
    case GS_Meniu:      Matrix_Animate();      break;
    case GS_InGame:     Matrix_InGame();      break;
    case GS_EndGame:    Matrix_Animate();    break;
    case GS_Hs:         Matrix_Animate();     break;
    case GS_HsSaveName: Matrix_Animate(); break;
    default:            gameState = GS_Meniu;               break; // it shoudn't be here    
  }
}
