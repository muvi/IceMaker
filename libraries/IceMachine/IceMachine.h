#ifndef IceMachine_h
#define IceMachine_h

#include "Arduino.h"
#include "limits.h"

#define TIMEOFF ULONG_MAX

#define STATEFLAG_CHANGEON_EVAPORATORCOLD 2
#define STATEFLAG_CHANGEON_EVAPORATORHOT 3
#define STATEFLAG_CHANGEON_BINEMPTY 24
#define STATEFLAG_CHANGEON_BINFULL 16
#define STATEFLAG_INCCOUNTER 64

#define RELAIS_COMPRESSOR 8
#define RELAIS_WATER 4
#define RELAIS_PUMP 2
#define RELAIS_HOTGAS 1

#define NUMICEMODES 15

struct __attribute__ ((packed)) IceState
{
  unsigned long time;
  char flags;
  //Compressor, Water, Pump, Hot Gas
  char relaisState;
  char nextState;
  unsigned long reserved;
  char description[21];
};

struct __attribute__ ((packed)) IceModes
{
  IceState states[NUMICEMODES];
  char Mode2Entry;
};


const IceModes defaultIceModes = {{
  {TIMEOFF, STATEFLAG_CHANGEON_EVAPORATORCOLD, 10 /*Compressor + Pump*/           , 1, 0, "Cooling Down"},
  {600000 , 0                                , 10 /*Compressor + Pump*/           , 2, 0, "Making Ice"},
  {TIMEOFF, STATEFLAG_CHANGEON_EVAPORATORHOT , 9 /*Compressor + Hot Gas*/         , 3, 0, "Preparing to drop"},
  {TIMEOFF, STATEFLAG_CHANGEON_BINEMPTY      , 0                                  , 4, 0, "Container Full"},
  {60000  , 0                                , 13 /*Compressor + Water + Hot Gas*/, 5, 0, "Dropping + Refilling"},
  {40000  , STATEFLAG_INCCOUNTER             , 11 /*Compressor + Pump + Hot Gas*/ , 0, 0, "Dropping Ice"},

  {TIMEOFF, STATEFLAG_CHANGEON_EVAPORATORHOT , 9 /*Compressor + Hot Gas*/         , 7, 0, "Defrosting"},
  {120000 , 0                                , 2 /*Pump*/                         , 8, 0, "Pumping"},
  {60000  , 0                                , 6 /*Pump + Water*/                 , 7, 0, "Refilling Water"},
  {0}, {0}, {0}, {0}, {0}, {0}//, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}
}, 6};

class IceMachine
{
  public:
    IceMachine(int pinHotGas, int pinPump, int pinWater, int pinCompressor, int pinBinThermostat, int pinEvaporatorThermostat);
    void proceed();
    void setMode(char mode);
    char getMode();
    long getTimeLeft();
    unsigned int getBatchesProduced();
    IceState* getState();
    IceModes modes;
  private:
    int _pinHotGas;
    int _pinPump;
    int _pinWater;
    int _pinCompressor;
    int _pinBinThermostat;
    int _pinEvaporatorThermostat;

    unsigned int _batchesProduced;
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
