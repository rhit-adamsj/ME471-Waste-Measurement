#include <LiquidCrystal.h>
#include <HX711.h>
#include <DS3231.h>
#include <Wire.h>
#include <avr/sleep.h>

float load1; float load2; float load3; float load4; float currentLoad;

DS3231 myRTC;
byte year; byte month; byte date; byte nDoW; String dOW;
byte hour; byte minute; byte second;
byte tempC;
bool century; bool h12Flag; bool pmFlag;

// Variables for use in method parameter lists
byte alarmDay;
byte alarmHour;
byte alarmMinute;
byte alarmSecond;
byte alarmBits;
bool alarmDayIsDay;
bool alarmH12;
bool alarmPM;

float prevLoad;
bool unloaded = false;
volatile bool alarmFlag = false;

HX711 cell1; 
#define LOADCELL1_SCK_PIN 24
#define LOADCELL1_DOUT_PIN 4
HX711 cell2;
#define LOADCELL2_SCK_PIN 25
#define LOADCELL2_DOUT_PIN 5
HX711 cell3;
#define LOADCELL3_SCK_PIN 26
#define LOADCELL3_DOUT_PIN 6
HX711 cell4;
#define LOADCELL4_SCK_PIN 27
#define LOADCELL4_DOUT_PIN 7

#define wakePin 2

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

void setup() {
  // Start the serial port
  Serial.begin(57600);
    
  // Start the I2C interface
  Wire.begin();

  // Attach load cells to their respective input pins
  cell1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  cell2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
  cell3.begin(LOADCELL3_DOUT_PIN, LOADCELL3_SCK_PIN);
  cell4.begin(LOADCELL4_DOUT_PIN, LOADCELL4_SCK_PIN);

  // Apply scaling factor and zero each load cell
  cell1.set_scale(2000.f);
  cell1.tare();
  cell2.set_scale(2000.f);
  cell2.tare();
  cell3.set_scale(2000.f);
  cell3.tare();
  cell4.set_scale(2000.f);
  cell4.tare();
  
  lcd.begin(16, 2);

  // When using interrupt with only one of the DS3231 alarms, as in this example,
  // it may be possible to prevent the other alarm entirely,
  // so it will not covertly block the outgoing interrupt signal.

  // Try to prevent Alarm 2 altogether by assigning a 
  // nonsensical alarm minute value that cannot match the clock time,
  // and an alarmBits value to activate "when minutes match".
  alarmMinute = 0xFF; // a value that will never match the time
  alarmBits = 0b01100000; // Alarm 2 when minutes match, i.e., never
  
  // Upload the parameters to prevent Alarm 2 entirely
  myRTC.setA2Time(
      alarmDay, alarmHour, alarmMinute,
      alarmBits, alarmDayIsDay, alarmH12, alarmPM);
  // disable Alarm 2 interrupt
  myRTC.turnOffAlarm(2);
  // clear Alarm 2 flag
  myRTC.checkIfAlarm(2);

  // Disable Analog Comparator
  // ACSR |= _BV(ACD);

  // Disable ADC via and set corresponding power reduction mode
  // ADCSRA &= ~_BV(ADEN);
  // PRR1 |= _BV(PRADC);
}

void loop() {
  load1 = calculateLoad(cell1);
  Serial.println(load1);
  load2 = calculateLoad(cell2);
  Serial.println(load2);
  load3 = calculateLoad(cell3);
  Serial.println(load3);
  load4 = calculateLoad(cell4);
  Serial.println(load4);

  currentLoad = load1+load2+load3+load4;

  cell1.power_down();
  cell2.power_down();
  cell3.power_down();
  cell4.power_down();

  /*if (currentLoad - prevLoad)/prevLoad < -0.5 {
    // If relative change in weight shows more than a 50% decrease from last measurement, do things
    unloaded = true;
    
  }*/

  getRTCValues();
  Serial.print("Time: "); Serial.print(dOW + ", ");
  Serial.print(month); Serial.print("/"); Serial.print(date); Serial.print("/"); Serial.print(year); Serial.print(" ");
  Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
  Serial.println();
  Serial.print("Total load = "); Serial.print(currentLoad); Serial.println(" lbs");
  updateLCD();
  
  // Powerdown until needed again
  goToSleep();

  cell1.power_up();
  cell2.power_up();
  cell3.power_up();
  cell4.power_up();
}

void handleUnload() {

}

float calculateLoad(HX711 cell) {
  /*Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);*/
  return cell.get_units();
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

void updateLCD(){
  lcd.setCursor(0, 0);
  lcd.print(currentLoad); 
  lcd.print(" lbs    ");

  lcd.setCursor(0, 1);
  lcd.print(month); lcd.print("/"); 
  lcd.print(date); lcd.print("/"); 
  lcd.print(year); lcd.print(" ");
  lcd.print(hour); lcd.print(":"); 
  lcd.print(minute/10); lcd.print(minute%10); lcd.print(":");
  lcd.print(second/10); lcd.print(second%10);
  lcd.print("   ");
}

void goToSleep() {
  
  alarmDay = myRTC.getDate();
  alarmHour = myRTC.getHour(alarmH12, alarmPM);
  alarmMinute = myRTC.getMinute() + 5;  // Set alarm1 for 5 minutes
  alarmSecond = myRTC.getSecond();
  alarmBits = 0b00001100; // Alarm 1 when minutes match
  alarmDayIsDay = false; // using date of month

  // Upload initial parameters of Alarm 1
  myRTC.turnOffAlarm(1);
  myRTC.setA1Time(
      alarmDay, alarmHour, alarmMinute, alarmSecond,
      alarmBits, alarmDayIsDay, alarmH12, alarmPM);
  // clear Alarm 1 flag after setting the alarm time
  myRTC.checkIfAlarm(1);
  // now it is safe to enable interrupt output
  myRTC.turnOnAlarm(1);

  // attach clock interrupt
  pinMode(wakePin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(wakePin), SleepISR, FALLING);  // Assign parameter values for Alarm 1

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void SleepISR() {

  // Disable interrupt pin to prevent repetition
  detachInterrupt(digitalPinToInterrupt(wakePin));
}
