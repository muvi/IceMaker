#include <SoftwareSerial.h>

SoftwareSerial BTSerial(2, 3);

bool state = false;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
}

void loop() {
  Serial.println("YAY");
  delay(1000);
  if (Serial.available() > 0) {
    int val = Serial.read();
    state = !state;
    digitalWrite(13, state);
    /*if (val == 'o') {
      digitalWrite(13, LOW);
    }
    if (val == 'i') {
      digitalWrite(13, HIGH);
    }*/
  }
}
