#include <LiquidCrystal.h>
#include <HX711.h>
#include <DS3231.h>
#include <Wire.h>

float load1; float load2; float load3; float load4; float totalLoad;

DS3231 myRTC;
byte year; byte month; byte date; byte nDoW; String dOW;
byte hour; byte minute; byte second;
byte tempC;
bool century; bool h12Flag; bool pmFlag;

HX711 cell1; 
const int LOADCELL1_DOUT_PIN = 4;
HX711 cell2;
const int LOADCELL2_DOUT_PIN = 5;
HX711 cell3;
const int LOADCELL3_DOUT_PIN = 6;
HX711 cell4;
const int LOADCELL4_DOUT_PIN = 7;
const int LOADCELL_SCK_PIN = 3;

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

void setup() {
  // Start the serial port
  Serial.begin(57600);
    
  // Start the I2C interface
  Wire.begin();

  cell1.begin(LOADCELL1_DOUT_PIN, LOADCELL_SCK_PIN);
  cell2.begin(LOADCELL2_DOUT_PIN, LOADCELL_SCK_PIN);
  cell3.begin(LOADCELL3_DOUT_PIN, LOADCELL_SCK_PIN);
  cell4.begin(LOADCELL4_DOUT_PIN, LOADCELL_SCK_PIN);

  cell1.set_scale(2000.f);
  cell1.tare();
  cell2.set_scale(2000.f);
  cell2.tare();
  cell3.set_scale(2000.f);
  cell3.tare();
  cell4.set_scale(2000.f);
  cell4.tare();

  lcd.begin(16, 2);
}

void loop() {
  load1 = calculateLoad(cell1);
  load2 = calculateLoad(cell2);
  load3 = calculateLoad(cell3);
  load4 = calculateLoad(cell4);

  totalLoad = load1+load2+load3+load4;

  cell1.power_down();
  cell2.power_down();
  cell3.power_down();
  cell4.power_down();

  getRTCValues();
  Serial.print("Time: "); Serial.print(dOW + ", ");
  Serial.print(month); Serial.print("/"); Serial.print(date); Serial.print("/"); Serial.print(year); Serial.print(" ");
  Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.print(second);
  Serial.println();
  Serial.print("Total load = "); Serial.print(totalLoad); Serial.println(" lbs");
  updateLCD();
  delay(1000);
  cell1.power_up();
  cell2.power_up();
  cell3.power_up();
  cell4.power_up();
}

float calculateLoad(HX711 scale) {
  /*Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 1);*/
  return scale.get_units();
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
  lcd.print(totalLoad); 
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
