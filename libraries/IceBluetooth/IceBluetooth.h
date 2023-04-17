#ifndef ICEBLUETOOTH_H
#define ICEBLUETOOTH_H

#include "IceMachine.h"
#include "IceEEPROM.h"
#include "SoftwareSerial.h"

class IceBluetooth
{
  public:
    IceBluetooth(IceMachine& iceMachine, IceEEPROM& iceEEPROM);
    void processMsgs();
  private:
    IceMachine& _iceMachine;
    IceEEPROM& _iceEEPROM;
    SoftwareSerial _btSerial;
    int receivingStateProgress = -1;
    void sendState();
    void sendStates();
    void setStates();
};

#endif
