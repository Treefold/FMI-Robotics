//  *DO NOT RENAME FILE*
//  the files names matter due to the compilling order

void setup() {
  Serial.begin (9600);
  Js_Init();
  Matrix_Init();
}

void loop() {
    Matrix_InGame();
}
