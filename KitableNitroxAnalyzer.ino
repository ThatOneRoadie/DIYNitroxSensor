/*****************************************************************************
*
* Kitable Nitrox Analyzer for Arduino Pro Mini
* Must alter pins if using any other arduino variant
* 
* License
* -------
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*****************************************************************************/

#include <SPI.h>  // may not be necessary
#include <Wire.h>  // may not be necessary
#include <Adafruit_SSD1306.h>
//#include <Adafruit_ADS1x15.h>
#include "ADS1X15.h"
#include <EEPROM.h>
#include <RunningAverage.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define O2_RA_SIZE 20
#define CO_RA_SIZE 5
RunningAverage RAO2(O2_RA_SIZE);
RunningAverage RACO(CO_RA_SIZE);

ADS1115 ads(0x48); 

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET

const int buttonPin=2; // push button
const int buzzer = 9; // buzzer
const int ledPin = 13; // led

double calibrationv;
float multiplier;

const int cal_holdTime = 2; // 2 sec button hold to calibration
const int mod_holdTime = 4; // 4 sec hold to po2 mod change
const int max_holdtime = 6; // 6 sec hold to reset max o2 result

long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
byte previous = HIGH;
unsigned long firstTime; // how long since the button was first pressed 
int active = 0;
double result_max = 0;

/*
 Calculate MOD (Maximum Operating Depth)
*/
float max_po1 = 1.40; //default setting on boot - changed to 1.40 which is more common
const float max_po2 = 1.60;
float cal_mod (float percentage, float ppo2 = 1.4) {
  return 32.8084 * ( (ppo2/(percentage/100)) - 1 );
}

//-----------------------------------------------------------------------------

void beep(int x=1) { // make beep for x time
  //digitalWrite(ledPin, HIGH); // led blink disable for battery save
  for(int i=0; i<x; i++) {    
      tone(buzzer, 2800, 100);
      delay(200);    
  }
  //digitalWrite(ledPin, LOW);
  noTone(buzzer);
}  // end beep function

//-----------------------------------------------------------------------------
//Oxygen Sensor Read
void read_o2_sensor(int adc=0) {  
  int16_t millivolts = 0;
  millivolts = ads.readADC_Differential_0_1();
  RAO2.addValue(millivolts);
}  // end read_o2_sensor function

//CO Sensor Read
void read_co_sensor(int adc=0) {  
  int16_t millivolts = 0;
  millivolts = ads.readADC(2);
  RACO.addValue(millivolts);
}  // end read_co_sensor function
//-----------------------------------------------------------------------------

void setup(void) {  

	Serial.begin(9600);

  /* power saving stuff for battery power */
  // Disable ADC
  // ADCSRA = 0;
  // Disable the analog comparator by setting the ACD bit
  // (bit 7) of the ACSR register to one.
  // ACSR = B10000000;
  // Disable digital input buffers on all analog input pins
  // DIDR0 = DIDR0 | B00111111;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  ads.setGain(2);
  multiplier = 0.0625F;
  ads.begin(); // ads1115 start
  
  pinMode(buttonPin,INPUT_PULLUP);  
  
  RAO2.clear();
  for(int cx=0; cx<= O2_RA_SIZE; cx++) {
     read_o2_sensor(0);
  }
    
  //calibrationv = EEPROMReadInt(0);  
  //if (calibrationv < 1) {
  //  calibrationv = calibrate(0);
  //}

  //Always calibrate on boot
  calibrationv = calibrate(0);
  
  beep(1);
}  // end setup

//-----------------------------------------------------------------------------

void EEPROMWriteInt(int p_address, int p_value)
     {
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

     EEPROM.write(p_address, lowByte);
     EEPROM.write(p_address + 1, highByte);
     }

//-----------------------------------------------------------------------------

unsigned int EEPROMReadInt(int p_address)
     {
     byte lowByte = EEPROM.read(p_address);
     byte highByte = EEPROM.read(p_address + 1);

     return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
     }

//-----------------------------------------------------------------------------

int calibrate(int x) {
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);  
  display.setTextSize(2);
  display.print(F("Calibrate"));
  display.display();
    
  //RAO2.clear();
  double result;  
  for(int cx=0; cx<= O2_RA_SIZE; cx++) {
    read_o2_sensor(0);
  }
  result = RAO2.getAverage();
  result = abs(result);
  EEPROMWriteInt(x, result); // write to eeprom
  
  // CO.clear();
  double coresult;
  for(int cx=0; cx<= CO_RA_SIZE; cx++) {
    read_co_sensor(0);
  }
  coresult = RACO.getAverage();
  coresult = abs(coresult);
  EEPROMWriteInt(x, coresult); // write to eeprom 

  beep(1);
  delay(1000);
  active = 0;
  return result;
}  // end calibrate function

// Helper to Center Text
void drawCentreString(const String &buf, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    display.setCursor(x - w / 2, y);
    display.print(buf);
}
//-----------------------------------------------------------------------------

void analysing(int x, int cal) {
  double currento2mv=0;
  double currentcomv=0;
  double result;
  double coresult;
  double o2mv = 0.0;
  double comv = 0.0;


  read_o2_sensor(0);
  currento2mv = RAO2.getAverage();
  currento2mv = abs(currento2mv);
  read_co_sensor(0);
  currentcomv = RACO.getAverage();
  currentcomv = abs(currentcomv);  

  
  result = (currento2mv / cal) * 20.9;
  if (result > 99.9) result = 99.9;
  o2mv = currento2mv * multiplier;

  comv = currentcomv * multiplier;
  coresult = (comv - 400) / 8;
  if (coresult < 0) coresult = 0;
  
 
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  if (o2mv < 0.02 || result <= 0) {
     display.setTextSize(2);
     display.println(F("Sensor"));
     display.print(F("Error!"));
  } else {
    display.setCursor(15,16);
    display.setTextSize(3);
    display.print(result,1);
    display.println(F("%"));

    if (result >= result_max) {
      result_max = result;
    }
    
    display.setTextSize(1);
    display.setCursor(0,3);
    display.setTextColor(BLACK, WHITE);    
    display.print(F("Max "));
    display.print(result_max,1);
    display.print(F("%   "));    
    //display.setCursor(75,31);
    display.print(o2mv,2);    
    display.print(F("mv"));
     
    if (active % 4) {
      display.setCursor(115,29);
      display.setTextColor(WHITE);
      display.print(F("."));
    }  
    
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,39);
    display.print(F("pO2 "));
    display.print(max_po1,1);
    display.print(F("  /  "));
    display.print(max_po2,1);
    display.print(F(" MOD"));

    display.setTextSize(1);
    //display.setCursor(0,49);
    //display.print(cal_mod(result_max,max_po1),0);
    //display.print(F("/"));
    //display.print(cal_mod(result_max,max_po2),0);
    //display.print(F("ft "));
    String modstr = String(int(cal_mod(result,max_po1))) + "/" + String(int(cal_mod(result,max_po2))) + " ft";
    drawCentreString(modstr,64,49);

    display.setTextSize(1);
    display.setCursor(0,57);
    display.setTextColor(BLACK, WHITE);    
    display.print(F("CO:"));
    display.print(coresult,1);
    display.print(F(" ppm "));    
    //display.setCursor(75,31);
    display.print(comv,0);    
    display.print(F("mv"));
    
    // menu
    if (secs_held < 7 && active > 16) {
      display.setTextSize(2);
      display.setCursor(0,31);
      display.setTextColor(BLACK, WHITE);      
      if (secs_held >= cal_holdTime && secs_held < mod_holdTime) {
        display.print(F("   CAL    "));
      }
      if (secs_held >= mod_holdTime && secs_held < max_holdtime) {
        display.print(F("   PO2    "));
      }
      if (secs_held >= max_holdtime && secs_held < 10) {
        display.print(F("   MAX    "));
      }     
    }  
    // CO Alarm
    if (coresult > 0){
      if (active % 3) {
        display.setCursor(0,16);
        display.setTextSize(3);
        display.setTextColor(BLACK, WHITE);
        display.println(F("CO WARN"));
        }
    }
  }
  display.display();
}  // end analysing function

//-----------------------------------------------------------------------------

void lock_screen(long pause = 5000) {
  beep(1);
  display.setTextSize(1);
  display.setCursor(0,37);  
  display.setTextColor(0xFFFF, 0);
  display.print(F("                "));
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0,37);
  display.print(F("======= LOCK ======="));
  display.display();
  for (int i = 0; i < pause; ++i) {   
    while (digitalRead(buttonPin) == HIGH) {
      }
   }
   active = 0;
}  // end lock_screen function

//-----------------------------------------------------------------------------

void po2_change() {  
  if (max_po1 == 1.3) max_po1 = 1.4;
  else if (max_po1 == 1.4) max_po1 = 1.5;
  else if (max_po1 == 1.5) max_po1 = 1.3;
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);  
  display.setTextSize(2);
  display.println(F("pO2 set"));
  display.print(max_po1);
  display.display();
  beep(1);   
  delay(1000);
  active = 0;  
}  // end po2_change function

//-----------------------------------------------------------------------------

void max_clear() {
  result_max = 0;
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);  
  display.setTextSize(2);
  display.println(F("Max result"));
  display.print(F("cleared"));
  display.display();
  beep(1);   
  delay(1000);
  active = 0;
}  // end max_clear function

//-----------------------------------------------------------------------------

void loop(void) {

  int current = digitalRead(buttonPin);
 
  if (current == LOW && previous == HIGH && (millis() - firstTime) > 200) {
    firstTime = millis();
    active = 17;
  }

  millis_held = (millis() - firstTime);
  secs_held = millis_held / 1000;

  if (millis_held > 2) {
    if (current == HIGH && previous == LOW) {
      if (secs_held <= 0) {
        lock_screen();
      }
      if (secs_held >= cal_holdTime && secs_held < mod_holdTime) {        
        calibrationv = calibrate(0);
      }
      if (secs_held >= mod_holdTime && secs_held < max_holdtime) {
        po2_change();
      }
      if (secs_held >= max_holdtime && secs_held < 12) {
        max_clear();
      }
    }
  }

  previous = current;
  prev_secs_held = secs_held;
  
  analysing(0,calibrationv);
  delay(200);
    
  active++;
}  // end loop function

// End Program