#include <DS3231.h>
#include <Wire.h>

DS3231 myRTC;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  while (myRTC.getMinute()) {

    
  }
}
