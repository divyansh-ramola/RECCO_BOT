#include <Arduino.h>

#define PUL 25
#define DIR 26
#define ENA 27

void setup() {

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);

  digitalWrite(ENA, LOW);   // enable driver
  digitalWrite(DIR, HIGH);  // set direction
}

void loop() {

  digitalWrite(PUL, HIGH);
  delayMicroseconds(20);   // speed control

  digitalWrite(PUL, LOW);
  delayMicroseconds(20);
}
