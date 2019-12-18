// Game states and meniu states can be change anywhere
// This file must have the name of its containing file

#define PAUSE_PIN 2

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
  MS_Brightness,
  MS_Hs,
  MS_InfoCreator,
  MS_InfoGit,
  MS_InfoRobotics,
  MS_LastState   // link to the next of the first state
} meniuState = MS_Start;

void Pause() {delay(1);} // creates an infinite loop of waiting
