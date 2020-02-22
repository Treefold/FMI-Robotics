#include <EEPROM.h>

void HsWrite (uint8_t user, uint8_t score) {
  EEPROM.write (user*sizeof(uint8_t), score);
}

uint8_t HsRead (uint8_t user) {
  return EEPROM.read (user*sizeof(uint8_t));
}

void HsReset () {
  for (uint8_t currHs = 0; currHs < 40; ++currHs) {
    HsWrite   (currHs, 0);
  }
}
