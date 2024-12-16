#include <DS3231.h>
#include <Wire.h>

int cellOne; cellTwo; cellThree; cellFour; totalLoad;

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
}

int calculateLoad() {
  int load;



  return load;
}
