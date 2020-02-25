#include "EEPROM.h"
#define RANKERS_NO  3
char name[NAME_LEN + 1] = "       ";

// the rank list is flexible so in order not to rewrite the eeprom every time it changes, it will placed last
// any modification made won't change the already saved data

uint8_t GetIdByRank (uint8_t rank = 1) {
  if (rank == 0 || rank > RANKERS_NO) {return 0;}
  return EEPROM[1023-(rank-1)];
}

uint8_t SetIdByRank (uint8_t rank, uint8_t id) {
  if (rank == 0 || rank > RANKERS_NO || (id & 0x80)) {return 0;}
  EEPROM[1023-(rank-1)] = id;
}

uint8_t GetScoreById (uint8_t id) {
  if (id & 0x80) {return 0;}
  return EEPROM [id * (SCORE_LEN + NAME_LEN)];
}

void SetScoreById (uint8_t id, uint8_t score) {
  if (id & 0x80) {return;}
  EEPROM [id * (SCORE_LEN + NAME_LEN)] = score;
}

uint8_t GetScoreByRank (uint8_t rank) {
  if (rank == 0 || rank > RANKERS_NO) {return 0xFF;}
  return GetScoreById (GetIdByRank(rank));
}

void EEPROM_GetUser (User &user) {
  if (user.id == 0) {return;}
  user.score = EEPROM [user.id * (SCORE_LEN + NAME_LEN)];
  for (uint8_t c = 0; c < NAME_LEN; ++ c) {
    user.name[c] = EEPROM [user.id * (SCORE_LEN + NAME_LEN) + SCORE_LEN + c];
  }
  user.name[NAME_LEN] = '\0';
}

void EEPROM_SetUser (User &user) {
  if (user.id == 0) {return;}
  EEPROM [user.id * (SCORE_LEN + NAME_LEN)] = user.score;
  for (uint8_t c = 0; c < NAME_LEN; ++ c) {
    EEPROM [user.id * (SCORE_LEN + NAME_LEN) + SCORE_LEN + c] = user.name[c];
  }
}

void EEPROM_Clean() {
  for (uint32_t i = 0; i < 1024; ++i) {
    EEPROM[i] = 0;
  }
}

void PrintUser(User &user) {
  Serial.print("ID: ");
  Serial.println(user.id);
  Serial.print("Score: ");
  Serial.println(user.score);
  Serial.print("Name: ");
  Serial.println(user.name);
}
