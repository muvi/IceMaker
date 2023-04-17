#include <SoftwareSerial.h>
#include <IceBluetooth.h>
#include <IceEEPROM.h>
#include <IceMachine.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

//pins
const int pinHotGas = 4;
const int pinPump = 5;
const int pinWater = 6;
const int pinCompressor = 3;
const int pinBinThermostat = 8;
const int pinEvaporatorThermostat = 7;
const int pinLED = 13;

//times
const long timeWaitForCleanup = 2500;
const long displayDelayTime = 1000;

IceMachine iceMachine = IceMachine(pinHotGas, pinPump, pinWater, pinCompressor, pinBinThermostat, pinEvaporatorThermostat);
IceEEPROM* iceEEPROM = NULL;
IceBluetooth* bt = NULL;

//cleanup mode
const byte magicCleanup = 42;
const int cleanupAddress = 0;
boolean cleanupMode;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void displayState(String state) {
  while (state.length() < 20) {
    state += " ";
  }
  lcd.setCursor(0, 1);
  lcd.print(state);
}

void displayTime(long time){
  time /= 1000;
  String s = String(time % 60);
  if (s.length() == 1) {
    s = "0" + s;
  }
  s = String(time / 60) + ":" + s;
  while (s.length() < 20) {
    s += " ";
  }
  lcd.setCursor(0, 2);
  lcd.print(s);
}

void displayNoTime(){
  lcd.setCursor(0, 2);
  lcd.print("                    ");
}

void displayIceProduced(int iceProduced){
  lcd.setCursor(0, 3);
  String s = String(iceProduced) + " Batches produced";
  while (s.length() < 20) {
    s += " ";
  }
  lcd.print(s);
}

void setup() {
  pinMode(pinLED, OUTPUT);  

  lcd.begin(20,4);
  lcd.clear();
  lcd.home();
  
  //iceMachine.modes = defaultIceModes;
  iceEEPROM = new IceEEPROM(iceMachine.modes);
  if (iceEEPROM->isMode2()) {
    iceEEPROM->mode2Timeout();
    lcd.print("Wohnheim E Ice Maker");
    lcd.setCursor(0, 3);
    lcd.print("Cleaning mode       ");
    
    iceMachine.setMode(iceMachine.modes.Mode2Entry);
  } else {

    lcd.print("Wohnheim E Ice Maker");

    //check for cleanup mode
    delay(timeWaitForCleanup);
    
    iceEEPROM->mode2Timeout();
    
    lcd.setCursor(0,3);
    lcd.print("0 Batches produced");

    iceMachine.setMode(0);
  }
  
  bt = new IceBluetooth(iceMachine, *iceEEPROM);
}

unsigned long lastDisplay = 0;

void loop() {
  iceMachine.proceed();
  bt->processMsgs();
  
  unsigned long now = millis();
  if (now - lastDisplay >= 250) {
    lastDisplay = now;
    digitalWrite(pinLED, HIGH);
    
    if (iceMachine.getState()->time == TIMEOFF) {
      displayNoTime();
    } else {
      displayTime(iceMachine.getTimeLeft());
    }
    displayState(iceMachine.getState()->description);
    displayIceProduced(iceMachine.getBatchesProduced());
    
    digitalWrite(pinLED, LOW);
  }
  //delay(250);
}
