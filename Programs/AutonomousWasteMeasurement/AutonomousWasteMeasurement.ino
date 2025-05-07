#include <LiquidCrystal.h>
#include <HX711.h>
#include <DS3231.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Written by Jay Adams for Mechanical Engineering Capstone Design; Team 51

float load1; 
float load2; 
float load3; 
float load4; 
float currentLoad; float maxLoad; 
float prevLoad;
bool shouldTare = false;
bool forceTare = false;

volatile uint8_t mainEventFlags = 0;
#define ALARM_WAKEUP_FLAG 0x01
#define PUSHBUTTON_TARE_FLAG 0x02
#define PUSHBUTTON_LCD_FLAG 0x04

DS3231 myRTC;
byte year; byte month; byte date; byte nDoW; String dOW;
byte hour; byte minute; byte second;
byte tempC;
byte tempRef; 
bool century; bool h12Flag; bool pmFlag;

// Variables for use in method parameter lists
byte alarmDay;
byte alarmHour;
byte alarmMinute;
byte alarmSecond;
byte alarmBits;
bool alarmDayIsDay = false;   // using date of month
bool alarmH12 = false;
bool alarmPM = false; 
volatile bool alarmFlag = false;

HX711 cell1; 
#define LOADCELL1_SCK_PIN 24
#define LOADCELL1_DOUT_PIN 30
HX711 cell2;
#define LOADCELL2_SCK_PIN 25
#define LOADCELL2_DOUT_PIN 31
HX711 cell3;
#define LOADCELL3_SCK_PIN 26
#define LOADCELL3_DOUT_PIN 32
HX711 cell4;
#define LOADCELL4_SCK_PIN 27
#define LOADCELL4_DOUT_PIN 33

#define WAKE_PIN 2
#define BACKLIGHT_CONTROL_PIN 10
#define ALARM_LENGTH 1  // Minutes
#define MORNING_HOUR 8  // Time at which device will wake up after shutting down overnight
#define NIGHT_HOUR 18   // Time at which device will shut down for the night
#define PUSHBUTTON_TARE_PIN 19
#define PUSHBUTTON_LCD_PIN 18
#define DEBOUNCE_DELAY 20

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

void setup() {
  // Start the serial port
  Serial.begin(57600);
  // Start the I2C interface
  Wire.begin();

  // Implement power saving techniques
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A6, LOW);
  digitalWrite(A7, LOW);
  digitalWrite(A8, LOW);
  digitalWrite(A9, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A11, LOW);
  digitalWrite(A12, LOW);
  digitalWrite(A13, LOW);
  digitalWrite(A14, LOW);
  digitalWrite(A15, LOW);

  // Write all digital pins as LOW outputs
  for (int i = 0; i <= 53; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }

  // Disable Analog Comparator
  ACSR |= _BV(ACD);
  // Disable ADC via and set corresponding power reduction mode
  ADCSRA = 0;
  PRR0 |= _BV(PRADC);
  // PRR0 |= _BV(PRUSART0); // Disables Serial outputs for debugging, but seems to operate fine --> Comment this line when testing
  PRR0 |= _BV(PRSPI);
  PRR0 |= _BV(PRTIM1);
  // PRR0 |= _BV(PRTIM0); // DONT TOUCH
  PRR0 |= _BV(PRTIM2);

  PRR1 |= _BV(PRUSART1);    
  PRR1 |= _BV(PRUSART2); 
  PRR1 |= _BV(PRUSART3);
  PRR1 |= _BV(PRTIM3);
  PRR1 |= _BV(PRTIM4);
  PRR1 |= _BV(PRTIM5);

  // Re-enable necessary pins --> AVOID USING DIGITAL 13 AS LED CONSUMES POWER
  pinMode(WAKE_PIN, INPUT_PULLUP);
  pinMode(LOADCELL1_DOUT_PIN, INPUT);
  pinMode(LOADCELL2_DOUT_PIN, INPUT);
  pinMode(LOADCELL3_DOUT_PIN, INPUT);
  pinMode(LOADCELL4_DOUT_PIN, INPUT);
  pinMode(PUSHBUTTON_TARE_PIN, INPUT_PULLUP);
  pinMode(PUSHBUTTON_LCD_PIN, INPUT_PULLUP);

  digitalWrite(BACKLIGHT_CONTROL_PIN, HIGH);

  // Attach load cells to their respective input pins
  cell1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  cell2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);
  cell3.begin(LOADCELL3_DOUT_PIN, LOADCELL3_SCK_PIN);
  cell4.begin(LOADCELL4_DOUT_PIN, LOADCELL4_SCK_PIN);
  // Apply scaling factor and zero each load cell
  cell1.set_scale(1984.f); //1984 
  cell1.tare();
  cell2.set_scale(2003.f); //2003
  cell2.tare();
  cell3.set_scale(2013.f); //2013
  cell3.tare();
  cell4.set_scale(2040.f); //2040
  cell4.tare();

  lcd.begin(16, 2);
  
  // When using interrupt with only one of the DS3231 alarms, as in this example,
  // it may be possible to prevent the other alarm entirely,
  // so it will not covertly block the outgoing interrupt signal.

  // Try to prevent Alarm 2 altogether by assigning a 
  // nonsensical alarm minute value that cannot match the clock time,
  // and an alarmBits value to activate "when minutes match".
  
  // 0xFF = a value that will never match the time
  // 0b01100000 = Alarm 2 when minutes match, i.e., never
  
  // Upload the parameters to prevent Alarm 2 entirely
  myRTC.setA2Time(
      alarmDay, alarmHour, 0xFF,
      0b01100000, alarmDayIsDay, alarmH12, alarmPM);
  // disable Alarm 2 interrupt
  myRTC.turnOffAlarm(2);
  // clear Alarm 2 flag
  myRTC.checkIfAlarm(2);

  prevLoad = 0;
  interrupts();
}

void loop() {

  load1 = calculateLoad(cell1);
  cell1.power_down();
  load2 = calculateLoad(cell2);
  cell2.power_down();
  load3 = calculateLoad(cell3);
  cell3.power_down();
  load4 = calculateLoad(cell4);
  cell4.power_down();

  // Serial.print("Cell 1 = "); Serial.println(load1);
  // Serial.print("Cell 2 = "); Serial.println(load2);
  // Serial.print("Cell 3 = "); Serial.println(load3);
  // Serial.print("Cell 4 = "); Serial.println(load4);

  getRTCValues();
  currentLoad = load1+load2+load3+load4;
  if (currentLoad > maxLoad) {
    maxLoad = currentLoad;
  }

  if (currentLoad < -200) {
    // Very negative weight means dumpster must currently be off the scale
    // Display last measured load. --> Do not update the value or re-tare.
    Serial.println("Dumpster is not on scale!");
    updateLCD(prevLoad);
    shouldTare = false;
    // waitToTare = false; Useless?
  } else if (prevLoad > 20 && (currentLoad-prevLoad)/prevLoad < -0.5) {
    // If relative change in weight shows more than a 50% decrease from last measurement and is beyond threshold where noise could reasonably be the culprit,
    // Dumpster must have been unloaded and set back down on scale. --> Re-tare the scale and record the last measured load
    Serial.println("Dumpster has been emptied!");
    updateLCD(prevLoad);
    prevLoad = currentLoad;
    shouldTare = true;
    // waitToTare = false;
  } else {
    updateLCD(currentLoad);
    shouldTare = false;
    prevLoad = currentLoad;
  }
  
  Serial.print("Time: "); Serial.print(dOW + ", ");
  Serial.print(month); Serial.print("/"); Serial.print(date); Serial.print("/"); Serial.print(year); Serial.print(" ");
  Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
  Serial.println();
  Serial.print("Total load = "); Serial.print(currentLoad); Serial.println(" lbs");
  
  // Night Mode
  while (hour >= NIGHT_HOUR || hour < MORNING_HOUR) { 
    digitalWrite(BACKLIGHT_CONTROL_PIN, LOW);
    alarmHour = MORNING_HOUR;
    alarmMinute = 0;
    alarmSecond = 0;
    alarmBits = 0b00001000; // Alarm when hour, minute, and second match, i.e. on the hour specified
    goToSleep(); 
  }

  // Powerdown until needed again
  alarmHour = 0;
  alarmMinute = (minute + ALARM_LENGTH) % 60;
  alarmSecond = second;
  alarmBits = 0b00001100; // Alarm when minutes and seconds match, i.e. on the minute specified
  goToSleep();

  // delay(1000);

  // Check for LCD button pressed
  if (mainEventFlags & PUSHBUTTON_LCD_FLAG) {
    delay(DEBOUNCE_DELAY);
    if (!digitalRead(PUSHBUTTON_LCD_PIN))
    {
      digitalWrite(BACKLIGHT_CONTROL_PIN, !(digitalRead(BACKLIGHT_CONTROL_PIN)));
    }
    mainEventFlags &= !PUSHBUTTON_LCD_FLAG;
  }

  cell1.power_up();
  cell2.power_up();
  cell3.power_up();
  cell4.power_up();

  if (mainEventFlags & PUSHBUTTON_TARE_FLAG) {   // Check for tare button pressed
    delay(DEBOUNCE_DELAY);
    if (!digitalRead(PUSHBUTTON_TARE_PIN))
    {
      lcd.clear();
      cell1.tare(); cell2.tare(); cell3.tare(); cell4.tare();
      maxLoad = 0;
      tempRef = tempC;
      shouldTare = false;
    }
    mainEventFlags &= !PUSHBUTTON_TARE_FLAG;
  } else if (shouldTare) {  // Check for automated tare
    cell1.tare(); cell2.tare(); cell3.tare(); cell4.tare();
    tempRef = tempC;
    shouldTare = false;
  }
}

float calculateLoad(HX711 cell) {
  // TO DO: Add temperature compensation
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

void updateLCD(float totalLoad){
  lcd.setCursor(0, 0);
  lcd.print("Now: ");
  lcd.print(totalLoad); 
  lcd.print(" lbs    ");

  lcd.setCursor(0, 1);
  lcd.print("Max: ");
  lcd.print(maxLoad);
  lcd.print(" lbs    ");

  // lcd.setCursor(0, 1);
  // lcd.print(month); lcd.print("/"); 
  // lcd.print(date); lcd.print("/"); 
  // lcd.print(year); lcd.print(" ");
  // lcd.print(hour); lcd.print(":"); 
  // lcd.print(minute/10); lcd.print(minute%10); lcd.print(":");
  // lcd.print(second/10); lcd.print(second%10);
  // lcd.print("   ");
  Serial.print("Total Load = "); Serial.println(totalLoad);
}

void goToSleep() {
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
  Serial.println("Alarm set, going to sleep");
  delay(100);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(WAKE_PIN), SleepISR, LOW);  // Assign parameter values for Alarm 1
  EIFR |= _BV(digitalPinToInterrupt(WAKE_PIN)); // Clear interrupt flag for WAKE_PIN interrupt
  attachInterrupt(digitalPinToInterrupt(PUSHBUTTON_TARE_PIN), TareISR, FALLING);
  EIFR |= _BV(digitalPinToInterrupt(PUSHBUTTON_TARE_PIN)); // Clear interrupt flag for PUSHBUTTON_TARE_PIN interrupt
  attachInterrupt(digitalPinToInterrupt(PUSHBUTTON_LCD_PIN), lcdISR, FALLING);
  EIFR |= _BV(digitalPinToInterrupt(PUSHBUTTON_LCD_PIN)); // Clear interrupt flag for PUSHBUTTON_LCD_PIN interrupt
  interrupts();
  sleep_cpu();

  Serial.println("I'm awake!");
}

void SleepISR() {
  // Disable interrupt pin to prevent continuous interruptions
  detachInterrupt(digitalPinToInterrupt(WAKE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_TARE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_LCD_PIN));
}

void TareISR() {
  detachInterrupt(digitalPinToInterrupt(WAKE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_TARE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_LCD_PIN));
  Serial.println("FORCE TARE");
  mainEventFlags |= PUSHBUTTON_TARE_FLAG;
}

void lcdISR() {
  detachInterrupt(digitalPinToInterrupt(WAKE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_TARE_PIN));
  detachInterrupt(digitalPinToInterrupt(PUSHBUTTON_LCD_PIN));
  Serial.println("TOGGLE LCD");
  mainEventFlags |= PUSHBUTTON_LCD_FLAG;
}
