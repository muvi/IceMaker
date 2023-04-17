#include "Arduino.h"
#include "IceMachine.h"

#define STATECOOLDOWN 0
#define STATEMAKEICE 1
#define STATEWAITHOT 2
#define STATECONTAINERFULL 3
#define STATEDROPANDREFILL 4
#define STATEDROP 5

#define STATEDEFROST 0
#define STATECLEANUPPUMP 1
#define STATECLEANUPREFILL 2

IceState iceStates[] = {
  //id, time, changeOnBinState, changeOnEvaporatorState, relaisState, nextState, flags, description
  {STATECOOLDOWN, TIMEOFF, SENSOROFF, HIGH, 10 /*Compressor + Pump*/, 1, 0, "Cooling Down"},
  {STATEMAKEICE, 600000, SENSOROFF, SENSOROFF, 10 /*Compressor + Pump*/, 2, 0, "Making Ice"},
  {STATEWAITHOT, TIMEOFF, SENSOROFF, LOW, 9 /*Compressor + Hot Gas*/, 3, 0, "Preparing to drop"},
  {STATECONTAINERFULL, TIMEOFF, LOW, SENSOROFF, 0, 4, 0, "Container Full"},
  {STATEDROPANDREFILL, 60000, SENSOROFF, SENSOROFF, 13 /*Compressor + Water + Hot Gas*/, 5, 0, "Dropping + Refilling"},
  {STATEDROP, 40000, SENSOROFF, SENSOROFF, 11 /*Compressor + Pump + Hot Gas*/, 0, STATEFLAG_INCCOUNTER, "Dropping Ice"},

  {STATEDEFROST, TIMEOFF, SENSOROFF, LOW, 9 /*Compressor + Hot Gas*/, 7, 0, "Defrosting"},
  {STATECLEANUPPUMP, 120000, SENSOROFF, SENSOROFF, 2 /*Pump*/, 8, 0, "Pumping"},
  {STATECLEANUPREFILL, TIMEOFF, SENSOROFF, SENSOROFF, 6 /*Pump + Water*/, 7, 0, "Refilling Water"},
};

IceMachine::IceMachine(int pinHotGas, int pinPump, int pinWater, int pinCompressor, int pinBinThermostat, int pinEvaporatorThermostat, char mode) {
  _pinHotGas = pinHotGas;
  pinMode(pinHotGas, OUTPUT);
  _pinPump = pinPump;
  pinMode(pinPump, OUTPUT);
  _pinWater = pinWater;
  pinMode(pinWater, OUTPUT);
  _pinCompressor = pinCompressor;
  pinMode(pinCompressor, OUTPUT);
  _pinBinThermostat = pinBinThermostat;
  pinMode(pinBinThermostat, INPUT_PULLUP);
  _pinEvaporatorThermostat = pinEvaporatorThermostat;
  pinMode(pinEvaporatorThermostat, INPUT_PULLUP);

  _batchesProduced = 0;
  _mode = mode;
  _relaisState = 15; //1111 (initial state)
  setState(mode);
  retireState();
}

void IceMachine::setRelais(char state) {
  char stateChange = (_relaisState ^ state) & _relaisState;
  if (stateChange & 1) digitalWrite(_pinHotGas, HIGH);
  if ((stateChange >> 1) & 1) digitalWrite(_pinPump, HIGH);
  if ((stateChange >> 2) & 1) digitalWrite(_pinWater, HIGH);
  if ((stateChange >> 3) & 1) digitalWrite(_pinCompressor, HIGH);

  stateChange = (_relaisState ^ state) & state;
  if (stateChange & 1) digitalWrite(_pinHotGas, LOW);
  if ((stateChange >> 1) & 1) digitalWrite(_pinPump, LOW);
  if ((stateChange >> 2) & 1) digitalWrite(_pinWater, LOW);
  if ((stateChange >> 3) & 1) digitalWrite(_pinCompressor, LOW);

  _relaisState = state;
}

void IceMachine::setState(char state) {
  _state = &iceStates[state];
  _startTime = millis();
  if (_state->flags & STATEFLAG_INCCOUNTER) {
    _batchesProduced++;
  }
}

void IceMachine::retireState() {
  setRelais(_state->relaisState);
}

bool IceMachine::checkState() {
  if (millis() - _startTime >= _state->time) return true;
  if (_state->changeOnEvaporatorState == digitalRead(_pinEvaporatorThermostat)) return true;
  if (_state->changeOnBinState == digitalRead(_pinBinThermostat)) return true;
  return false;
}

void IceMachine::proceed() {
  if (checkState()) {
    do {
      setState(_state->nextState);
    } while (checkState());
    retireState();
  }
}

void IceMachine::setMode(char mode) {
  _mode = mode;
  setState(0);
  retireState();
}

char IceMachine::getMode() {
  return _mode;
}

long IceMachine::getTimeLeft() {
  return _state->time - (millis() - _startTime);
}

int IceMachine::getBatchesProduced() {
  return _batchesProduced;
}

IceState* IceMachine::getState() {
  return _state;
}

