#ifndef IceMachine_h
#define IceMachine_h

#include "Arduino.h"
#include "limits.h"

#define SENSOROFF 0xFF
#define TIMEOFF ULONG_MAX

#define MODE_MAKEICE 0
#define MODE_CLEANUP 6

#define STATEFLAG_INCCOUNTER 1

struct IceState
{
  char id;
  unsigned long time;
  char changeOnBinState;
  char changeOnEvaporatorState;
  //Compressor, Water, Pump, Hot Gas
  char relaisState;
  char nextState;
  char flags;
  String description;
};

class IceMachine
{
  public:
    IceMachine(int pinHotGas, int pinPump, int pinWater, int pinCompressor, int pinBinThermostat, int pinEvaporatorThermostat, char mode);
    void proceed();
    void setMode(char mode);
    char getMode();
    long getTimeLeft();
    int getBatchesProduced();
    IceState* getState();
  private:
    int _pinHotGas;
    int _pinPump;
    int _pinWater;
    int _pinCompressor;
    int _pinBinThermostat;
    int _pinEvaporatorThermostat;

    int _batchesProduced;
    unsigned long _startTime;
    char _mode;
    IceState* _state;
    char _relaisState;

    void setRelais(char state);
    void setState(char state);
    void retireState();
    bool checkState();
};

#endif
