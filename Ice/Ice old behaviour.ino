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

//machine modes: CWPH
const byte modeIdle = 0; //all off 0000
const byte modeMakeIce = 10; //Compressor + Pump 0101
const byte modeWaitHot = 9; //Compressor + Hot Gas
const byte modeDropIce = 13; //Compressor + Water + Hot Gas 1011
const byte modeCleanup = 2; //Pump
const byte modeCleanupWater = 6; //Pump + Water

//times
const long timeDrop = 120000;
const long timeProduce = 600000;
const long timeCleanup = 120000;
const long timeCleanupWater = 60000;
const long timeWaitForCleanup = 2500;

const long displayDelayTime = 1000;

//cleanup mode
const byte magicCleanup = 42;
const int cleanupAddress = 0;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
int cycles = 0;

void setMode(byte mode){
  digitalWrite(pinHotGas, mode & 1 ? LOW : HIGH);
  digitalWrite(pinPump, (mode >> 1) & 1 ? LOW : HIGH);
  digitalWrite(pinWater, (mode >> 2) & 1 ? LOW : HIGH);
  digitalWrite(pinCompressor, (mode >> 3) & 1 ? LOW : HIGH);
}

boolean binFull(){
  return digitalRead(pinBinThermostat) == HIGH;
}

boolean evaporatorHot(){
  return digitalRead(pinEvaporatorThermostat) == LOW;
}

void displayState(String state) {
  lcd.setCursor(0, 1);
  lcd.print(state);
}

void displayTime(long time){
  time /= 1000;
  String s = String(time % 60);
  if (s.length() == 1) {
    s = "0" + s;
  }
  s = String(time / 60) + ":" + s + "  ";
  lcd.setCursor(0, 2);
  lcd.print(s);
}

void displayNoTime(){
  lcd.setCursor(0, 2);
  lcd.print("                    ");
}

void displayIceProduced(){
  lcd.setCursor(0, 3);
  String s = String(cycles) + " Batches produced";
  while (s.length() < 20) {
    s += " ";
  }
  lcd.print(s);
}

void delayAndDisplay(long time){
  for (long i = 0; i < time; i += displayDelayTime) {
    displayTime(time - i);
    delay(displayDelayTime);
  }
  displayNoTime();
}

void cleanup(){
  //wait until hot
  displayState("Defrosting          ");
  
  setMode(modeWaitHot);
  while (!evaporatorHot()) {
    digitalWrite(pinLED, HIGH);
    delay(250);
    digitalWrite(pinLED, LOW);
    delay(250);
  }
  
  while (true) {
    //fill in water
    displayState("Refilling Water     ");
    setMode(modeCleanupWater);
    delayAndDisplay(timeCleanupWater);

    //cleanup
    displayState("Pumping             ");
    setMode(modeCleanup);
    delayAndDisplay(timeCleanup);
  }
}

void setup() {
  pinMode(pinHotGas, OUTPUT);
  pinMode(pinPump, OUTPUT);
  pinMode(pinWater, OUTPUT);
  pinMode(pinCompressor, OUTPUT);
  pinMode(pinBinThermostat, INPUT_PULLUP);
  pinMode(pinEvaporatorThermostat, INPUT_PULLUP);
  pinMode(pinLED, OUTPUT);  
  setMode(modeIdle);

  lcd.begin(20,4);
  lcd.clear();
  lcd.home();

  byte cleanupMode = EEPROM.read(cleanupAddress);
  if (cleanupMode == magicCleanup) {
    EEPROM.write(cleanupAddress, 0);
    lcd.print("Wohnheim E Ice Maker");
    cleanup();
  }
  EEPROM.write(cleanupAddress, magicCleanup);

  lcd.print("Wohnheim E Ice Maker");

  //check for cleanup mode
  delay(timeWaitForCleanup);
  EEPROM.write(cleanupAddress, 0);

  lcd.setCursor(0,3);
  lcd.print("0 Batches produced");
}

void loop() {  
  //wait until hot
  displayState("Preparing to drop   ");
  
  setMode(modeWaitHot);
  while (!evaporatorHot()) {
    digitalWrite(pinLED, HIGH);
    delay(250);
    digitalWrite(pinLED, LOW);
    delay(250);
  }
  
  //idle if bin is full
  if (binFull()) {
    displayState("Container full      ");
    
    setMode(modeIdle);
    digitalWrite(pinLED, HIGH);
    do {
      delay(500);
    } while (binFull());
    digitalWrite(pinLED, LOW);
  }

  //fill in water
  displayState("Dropping Ice        ");
  
  setMode(modeDropIce);
  delayAndDisplay(timeDrop);
  
  displayIceProduced();
  cycles++;
  
  //wait until cold
  displayState("Cooling Down        ");

  setMode(modeMakeIce);
  while (evaporatorHot()) {
    digitalWrite(pinLED, HIGH);
    delay(1000);
    digitalWrite(pinLED, LOW);
    delay(1000);
  }
 
  //make ice
  displayState("Making Ice          ");

  delayAndDisplay(timeProduce);
}
