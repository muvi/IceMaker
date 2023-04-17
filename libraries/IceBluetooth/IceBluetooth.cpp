#include "Arduino.h"
#include "IceBluetooth.h"
#include "IceMachine.h"

#define BT_MSGM_CHECK 'C'
#define BT_MSGM_QUERYSTATE 'Q'
#define BT_MSGM_QUERYSTATES 'Y'
#define BT_MSGM_SETSTATES 'S'

#define BT_MSGS_CHECKRESPONSE 'K'
#define BT_MSGS_STATEINFO 'I'
#define BT_MSGS_STATES 'S'
#define BT_MSGS_SETSTATESOK 'O'
#define BT_MSGS_SETSTATESERROR 'E'

union ULongSend {
  unsigned long ulong;
  byte sendVal[4];
};

union UIntSend {
  unsigned int uint;
  byte sendVal[2];
};

IceBluetooth::IceBluetooth(IceMachine& iceMachine, IceEEPROM& iceEEPROM) : _iceMachine(iceMachine), _iceEEPROM(iceEEPROM), _btSerial(11, 12) /*RX, TX*/ {
  _btSerial.begin(9600);
}

void IceBluetooth::processMsgs() {
  if (receivingStateProgress >= 0) {
    setStates();
    return;
  }
  if (_btSerial.available()) {
    switch (_btSerial.read()) {
      case BT_MSGM_CHECK: {
        _btSerial.write(BT_MSGS_CHECKRESPONSE);
        break;
      }
      case BT_MSGM_QUERYSTATE: {
        sendState();
        break;
      }
      case BT_MSGM_QUERYSTATES: {
        sendStates();
        break;
      }
      case BT_MSGM_SETSTATES: {
        receivingStateProgress = 0;
        setStates();
        break;
      }
    }    
  }
}

void IceBluetooth::sendState() {
  IceState* state = _iceMachine.getState();

  //id
  _btSerial.write(BT_MSGS_STATEINFO);
  //timeLeft
  ULongSend time;
  time.ulong = state -> time == TIMEOFF ? TIMEOFF : _iceMachine.getTimeLeft();
  _btSerial.write(time.sendVal[0]);
  _btSerial.write(time.sendVal[1]);
  _btSerial.write(time.sendVal[2]);
  _btSerial.write(time.sendVal[3]);
  //batches
  UIntSend batches;
  batches.uint = _iceMachine.getBatchesProduced();
  _btSerial.write(batches.sendVal[0]);
  _btSerial.write(batches.sendVal[1]);
  //description
  for (int i = 0; i < 21; i++) {
    _btSerial.write(state -> description[i]);
  }
}

void IceBluetooth::sendStates() {
  //id
  _btSerial.write(BT_MSGS_STATES);

  //states
  char* sendVal = (char*) &_iceMachine.modes;
  for (int i = 0; i < sizeof(IceModes); i++) {
    _btSerial.write(sendVal[i]);
  }
}

void IceBluetooth::setStates() {
  char* readVal = (char*) &_iceMachine.modes;
  int val;
  while (_btSerial.available()) {
    val = _btSerial.read();
    readVal[receivingStateProgress] = val;
    receivingStateProgress++;
    if (receivingStateProgress >= sizeof(IceModes)) {
      _btSerial.write(BT_MSGS_SETSTATESOK);
      receivingStateProgress = -1;
      _iceEEPROM.writeModes();
      break;
    }
  }
}

