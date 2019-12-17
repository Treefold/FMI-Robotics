void setup() {
  Serial.begin (9600);
  pinMode (PAUSE_PIN,   INPUT_PULLUP);
  pinMode (HS_resetPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PAUSE_PIN),   Pause,    LOW);
  attachInterrupt(digitalPinToInterrupt(HS_resetPin), HS_Reset, FALLING);
  Js_Init();
  Lcd_Init();
  Matrix_Init();
}

void loop() {
  switch (gameState) {
    case GS_Meniu:      Matrix_Animate(); Lcd_Meniu();      break;
    case GS_InGame:     Matrix_InGame();  Lcd_InGame();     break;
    case GS_EndGame:    Matrix_Animate(); Lcd_EndGame();    break;
    case GS_Hs:         Matrix_Animate(); Lcd_HsMeniu();    break;
    case GS_HsSaveName: Matrix_Animate(); Lcd_HsSaveName(); break;
    default:            gameState = GS_Meniu;               break; // it shoudn't be here    
  }
}
