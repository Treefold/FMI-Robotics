// Highscore HS
#include <EEPROM.h>
#define NAME_LEN (uint8_t) 8 // (bytes)
#define HS_TYPE            uint16_t
#define HS_LEN   (uint8_t) 2 // (bytes)

char    HS_Name [NAME_LEN];
HS_TYPE score = 0;

enum {
  HS_ADR   = 0,
  NAME_ADR = HS_ADR + HS_LEN
};

HS_TYPE HS_Read () {
  HS_TYPE score = 0;
  for (uint8_t currByte = 0; currByte < HS_LEN; ++currByte) {
    score = (score << 8) + EEPROM.read (HS_ADR + currByte);
  }
  return score;
}

void HS_Write (HS_TYPE score) {
  for (uint8_t currByte = HS_LEN; currByte > 0; --currByte) {
    EEPROM.write (HS_ADR + currByte - 1, (score & 0xFF));
    score >>= 8;
  }
}

void NAME_Read (char name[NAME_LEN]) {
  for (uint8_t currByte = 0; currByte < NAME_LEN - 1; ++currByte) {
    name[currByte] = EEPROM.read (NAME_ADR + currByte);
    // Serial.println( (int) EEPROM.read (NAME_ADR + currByte));
  }
  name[NAME_LEN - 1] = '\0';
}

void NAME_Write (char name[NAME_LEN]) {
  uint8_t currByte = 0;
  while (currByte < NAME_LEN && name[currByte] != '\0') {
    // Serial.print (name[currByte]);
    EEPROM.write (NAME_ADR + currByte, name[currByte]);
    currByte++;
  }
  while (currByte < NAME_LEN) {
    EEPROM.write (NAME_ADR + (currByte++), '\0');
  }
}

void HS_Update (HS_TYPE newScore, char name[NAME_LEN], bool ignoreMax)
{
  HS_TYPE lastScore = HS_Read();
  if (newScore > lastScore || ignoreMax) {
    HS_Write   (newScore);
    NAME_Write (name);
  }
}
