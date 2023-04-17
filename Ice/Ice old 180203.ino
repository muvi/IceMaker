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
const byte modeDropIceWater = 13; //Compressor + Water + Hot Gas 1011
const byte modeDropIcePump = 11; //Compressor + Pump + Hot Gas 1011
const byte modeCleanup = 2; //Pump
const byte modeCleanupWater = 6; //Pump + Water

//all relais are on by default
byte currentMode = 15;

//times
const long timeDropIceWater = 60000;
const long timeDropIcePump = 40000;
const long timeProduce = 720000;
const long timeCleanup = 120000;
const long timeCleanupWater = 60000;
const long timeWaitForCleanup = 2500;

const long displayDelayTime = 1000;

//cleanup mode
const byte magicCleanup = 42;
const int cleanupAddress = 0;
boolean cleanupMode;

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
int cycles = 0;

void setMode(byte mode){
  //notice: arduino tends to hang if a channel is switched on
  //when no other one was switched off directly before
  //switch off first
  //switch on
  byte modeChange = (currentMode ^ mode) & currentMode;
  if (modeChange & 1) digitalWrite(pinHotGas, HIGH);
  if ((modeChange >> 1) & 1) digitalWrite(pinPump, HIGH);
  if ((modeChange >> 2) & 1) digitalWrite(pinWater, HIGH);
  if ((modeChange >> 3) & 1) digitalWrite(pinCompressor, HIGH);

  modeChange = (currentMode ^ mode) & mode;
  if (modeChange & 1) digitalWrite(pinHotGas, LOW);
  if ((modeChange >> 1) & 1) digitalWrite(pinPump, LOW);
  if ((modeChange >> 2) & 1) digitalWrite(pinWater, LOW);
  if ((modeChange >> 3) & 1) digitalWrite(pinCompressor, LOW);


  
  currentMode = mode;
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
  //giving power supply time to stabilize
  displayState("Initializing        ");
  delay(1000);
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
    //cleanup
    displayState("Pumping             ");
    setMode(modeCleanup);
    delayAndDisplay(timeCleanup);

    //fill in water
    displayState("Refilling Water     ");
    setMode(modeCleanupWater);
    delayAndDisplay(timeCleanupWater);
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

  cleanupMode = (EEPROM.read(cleanupAddress) == magicCleanup);
  if (cleanupMode) {
    EEPROM.write(cleanupAddress, 0);
    lcd.print("Wohnheim E Ice Maker");
    lcd.setCursor(0, 3);
    lcd.print("Cleaning mode       ");
  } else {
    EEPROM.write(cleanupAddress, magicCleanup);

    lcd.print("Wohnheim E Ice Maker");

    //check for cleanup mode
    delay(timeWaitForCleanup);
    EEPROM.write(cleanupAddress, 0);

    lcd.setCursor(0,3);
    lcd.print("0 Batches produced");
  }
}

void loop() {
  if (cleanupMode) {
    cleanup();
  } else while (true) {
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
    displayState("Dropping + Refilling");
    setMode(modeDropIceWater);
    delayAndDisplay(timeDropIceWater);

    displayState("Dropping Ice        ");
    setMode(modeDropIcePump);
    delayAndDisplay(timeDropIcePump);
  
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
}
