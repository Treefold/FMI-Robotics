void setup() {
  Serial.begin (9600);
  Js_Init();
  Lcd_Init();
  Matrix_Init();
  NAME_Read (HS_Name);
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
