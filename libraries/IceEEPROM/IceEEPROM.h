#ifndef ICEEPROM_H
#define ICEEPROM_H

#include "IceMachine.h"


class IceEEPROM
{
  public:
    IceEEPROM(IceModes& iceModes);
    bool isMode2();
    void writeModes();
    void mode2Timeout();
  private:
    IceModes& _iceModes;
    bool _mode2;
    void readModes();
};

#endif
