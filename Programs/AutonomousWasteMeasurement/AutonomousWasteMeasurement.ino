#include <DS3231.h>
#include <Wire.h>

int cellOne; int cellTwo; int cellThree; int cellFour; int totalLoad;

DS3231 myRTC;

void setup() {
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
}

void loop() {
  cellOne = calculateLoad(digitalRead(6));
  cellTwo = calculateLoad(digitalRead(7));
  cellThree = calculateLoad(digitalRead(8));
  cellFour = calculateLoad(digitalRead(9));
  totalLoad = cellOne + cellTwo + cellThree + cellFour;

  // SMCR: Sleep mode control register
  SMCR |= _BV(SE);  // Enables sleep mode
  SMCR |= _BV(SM2); // Configures sleep mode
  SMCR |= _BV(SM1);
  SMCR |= _BV(SM0);

  PRR0 = 0; // Power reduction register 0
  PRR1 = 0; // Power reduction register 1
}

int calculateLoad(int digitalIn) {
  int load = 0;

  return load;
}
