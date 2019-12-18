// Highscore HS
#include <EEPROM.h>
#define NAME_LEN (uint8_t) 8 // (bytes)
#define HS_TYPE            uint16_t
#define HS_LEN   (uint8_t) 2 // (bytes)
#define HS_NO    (uint8_t) 3 // number of saved highscores
const uint8_t hs_resetPin = 3;

HS_TYPE HS_Read (uint8_t no) {
  HS_TYPE score = 0;
  for (uint8_t currByte = 0; currByte < HS_LEN; ++currByte) {
    score = (score << 8) + EEPROM.read ((HS_LEN + NAME_LEN) * no + currByte);
  }
  return score;
}

void HS_Write (HS_TYPE score, uint8_t no) {
  for (uint8_t currByte = HS_LEN; currByte > 0; --currByte) {
    EEPROM.write ((HS_LEN + NAME_LEN) * no + currByte - 1, (score & 0xFF));
    score >>= 8;
  }
}

void NAME_Read (char name[NAME_LEN], uint8_t no) {
  for (uint8_t currByte = 0; currByte < NAME_LEN - 1; ++currByte) {
    name[currByte] = EEPROM.read (((no + 1) * HS_LEN + no * NAME_LEN) + currByte);
  }
  name[NAME_LEN - 1] = '\0';
}

void NAME_Write (char name[NAME_LEN], uint8_t no) {
  uint8_t currByte = 0;
  while (currByte < NAME_LEN && name[currByte] != '\0') {
    EEPROM.write (((no + 1) * HS_LEN + no * NAME_LEN) + currByte, name[currByte]);
    currByte++;
  }
  while (currByte < NAME_LEN) {
    EEPROM.write (((no + 1) * HS_LEN + no * NAME_LEN) + (currByte++), '\0');
  }
}

void HS_Reset () {
  HS_TYPE scoreHS = 0;
  char    nameHS[NAME_LEN]  = "NOT SET";
  for (uint8_t currHS = 0; currHS < HS_NO; ++currHS) {
    HS_Write   (scoreHS, currHS);
    NAME_Write (nameHS,  currHS);
  }
}

void HS_Update (HS_TYPE newScoreHS, char newNameHS[NAME_LEN])
{
  if (newScoreHS > HS_Read (HS_NO - 1)) { // last HS; HS[0, HS_NO-1]
    char nameHS[NAME_LEN];
    bool stop = false;
    uint8_t currHS;
    for (currHS = HS_NO - 1; currHS > 0 && !stop; --currHS) {
      if (newScoreHS > HS_Read (currHS - 1)) { // update ranking
        NAME_Read  (nameHS,            currHS - 1);
        NAME_Write (nameHS,            currHS);
        HS_Write   (HS_Read(currHS-1), currHS);  
      }
      else {stop = true;break;}
    } 
    // insert HS
    HS_Write   (newScoreHS, currHS);
    NAME_Write (newNameHS,  currHS);
  }
}
