#include "Arduino.h"
#include "IceEEPROM.h"
#include "EEPROM.h"

#define EEPROM_MAGIC_MODE1 10
#define EEPROM_MAGIC_MODE2 20

#define EEPROM_MAGIC_ADDR 0
#define EEPROM_ICEMODE_ADDR 32

IceEEPROM::IceEEPROM(IceModes& iceModes) : _iceModes(iceModes) {
  byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  switch(magic) {
    case EEPROM_MAGIC_MODE1: {
      EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_MODE2);
      //continue to readModes();
    }
    case EEPROM_MAGIC_MODE2: {
      readModes();
      break;
    }
    default: {
      EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_MODE2);
      _iceModes = defaultIceModes;
      writeModes();
    }
  }
  _mode2 = (magic == EEPROM_MAGIC_MODE2);
}

void IceEEPROM::mode2Timeout() {
  EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_MODE1);
}

bool IceEEPROM::isMode2() {
  return _mode2;
}

void IceEEPROM::writeModes() {
  EEPROM.put(EEPROM_ICEMODE_ADDR, _iceModes);
}

void IceEEPROM::readModes() {
  EEPROM.get(EEPROM_ICEMODE_ADDR, _iceModes);
}

