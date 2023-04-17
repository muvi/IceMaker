#include "Arduino.h"
#include "IceMachine.h"

#define EVAPORATORSTATE(a) (~a & 3)
#define BINSTATE(a) ((~a & 24) >> 3)

IceMachine::IceMachine(int pinHotGas, int pinPump, int pinWater, int pinCompressor, int pinBinThermostat, int pinEvaporatorThermostat) {
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
  _mode = 0;
  _relaisState = 15; //1111 (initial state)
  setRelais(0);
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
  _state = &modes.states[state];
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
  if (EVAPORATORSTATE(_state->flags) == digitalRead(_pinEvaporatorThermostat)) return true;
  if (BINSTATE(_state->flags) == digitalRead(_pinBinThermostat)) return true;
  return false;
}

void IceMachine::proceed() {
  if (checkState()) {
    int i = 0;
    do {
      setState(_state->nextState);
      i++;
      if (i > NUMICEMODES) {
        //infinite loop detected
        return;
      }
    } while (checkState());
    retireState();
  }
}

void IceMachine::setMode(char mode) {
  _mode = mode;
  setState(mode);
  retireState();
}

char IceMachine::getMode() {
  return _mode;
}

long IceMachine::getTimeLeft() {
  return _state->time - (millis() - _startTime);
}

unsigned int IceMachine::getBatchesProduced() {
  return _batchesProduced;
}

IceState* IceMachine::getState() {
  return _state;
}

