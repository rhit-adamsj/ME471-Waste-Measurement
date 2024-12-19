#include <DS3231.h>
#include <Wire.h>

int cellOne; int cellTwo; int cellThree; int cellFour; int totalLoad;

DS3231 myRTC;
byte year; byte month; byte date; byte nDoW; String dOW;
byte hour; byte minute; byte second;
byte tempC;
bool century; bool h12Flag; bool pmFlag;

float VCC = 9;

void setup() {
  // Start the serial port
    Serial.begin(57600);
    
  // Start the I2C interface
  Wire.begin();
}

void loop() {
  /*cellOne = calculateLoad(digitalRead(6));
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
  PRR1 = 0; // Power reduction register 1*/
  int V0 = analogRead(0);
  int V1 = analogRead(1);
  Serial.print(V1); Serial.println(V0);
  getRTCValues();
  Serial.print("Time: "); Serial.print(dOW + ", ");
  Serial.print(month); Serial.print("/"); Serial.print(date); Serial.print("/"); Serial.print(year); Serial.print(" ");
  Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
  Serial.println();
  delay(5000);
}

int calculateLoad(int digitalIn) {
  int load = 0;

  return load;
}

void getRTCValues() {
  year = myRTC.getYear();
  month = myRTC.getMonth(century);
  date = myRTC.getDate();
  nDoW = myRTC.getDoW();
  switch (nDoW) {
    case 1:
      dOW = "Sunday";
      break;
    case 2:
      dOW = "Monday";
      break;
    case 3:
      dOW = "Tuesday";
      break;
    case 4:
      dOW = "Wednesday";
      break;
    case 5:
      dOW = "Thursday";
      break;
    case 6:
      dOW = "Friday";
      break;
    case 7:
      dOW = "Saturday";
      break;

  }
  hour = myRTC.getHour(h12Flag, pmFlag);
  minute = myRTC.getMinute();
  second = myRTC.getSecond();
  tempC = myRTC.getTemperature();
}
