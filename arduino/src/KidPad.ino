// LCD
#include "U8glib.h"


/* Digital pin number for the displays chip select pin */
#define CS_DI 10
/* Digital pin number for the displays data/command pin */
#define DC_DI 9
/* Digital pin number for the displays -reset pin */
#define RST_DI 8

#define RED_DI 26
#define YELLOW_DI 25
#define GREEN_DI 24
#define BLUE_DI 23
#define BLACK_DI 22

//D0 D1 CS A0 RESET
U8GLIB_SSD1306_128X64 u8g(7, 6, 5, 3, 2);

// ROTARY

int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long lastStateChangeTime = 0;
int cleared = 0;
short digit_1;
short digit_2;
short operation = 0;
String words[40];
String colors[5];
short pans[40];
short color_pans[5];
boolean started = false;
boolean ask = true;
boolean color_mode = false;

#define ROTARY_IN 4
#define DOOR_1 8
#define FINISHED_AFTER_MS 100
#define DEBOUNCE_DELAY 10


void setup() 
{

  // LCD
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }

  // COLOR

  pinMode(RED_DI,INPUT);
  digitalWrite(RED_DI,HIGH);
  pinMode(YELLOW_DI,INPUT);
  digitalWrite(YELLOW_DI,HIGH);
  pinMode(GREEN_DI,INPUT);
  digitalWrite(GREEN_DI,HIGH);
  pinMode(BLUE_DI,INPUT);
  digitalWrite(BLUE_DI,HIGH);
  pinMode(BLACK_DI,INPUT);
  digitalWrite(BLACK_DI,HIGH);
  
  // ROTARY
  
  pinMode(ROTARY_IN, INPUT);

  // DOOR
  randomSeed(analogRead(0));
  pinMode(DOOR_1, OUTPUT);
  words[0] = "\xBD\xBE\xBB\xCC";
  pans[0] = 20;
  words[1] = "\xBE\xB4\xB8\xBD";
  pans[1] = 20;
  words[2] = "\xB4\xB2\xB0";
  pans[2] = 30;
  words[3] = "\xC2\xC0\xB8";
  pans[3] = 30;
  words[4] = "\xC7\xB5\xC2\xCB\xC0\xB5";
  pans[4] = 0;
  words[5] = "\xBF\xCF\xC2\xCC";
  pans[5] = 20;
  words[6] = "\xC8\xB5\xC1\xC2\xCC";
  pans[6] = 10;
  words[7] = "\xC1\xB5\xBC\xCC";
  pans[7] = 20;
  words[8] = "\xB2\xBE\xC1\xB5\xBC\xCC";
  pans[8] = 0;
  words[9] = "\xB4\xB5\xB2\xCF\xC2\xCC";
  pans[9] = 0;


  words[10] = "";
  pans[10] = 10;
  words[11] = "\xB0\xC0\xB1\xC3\xB7";
  pans[11] = 10;
  words[12] = "\xB1\xB0\xB1\xBE\xC7\xBA\xB0";
  pans[12] = 0;
  words[13] = "\xD2\xD8\xDD\xDE\xD3\xE0\xD0\xD4";
  pans[13] = 10;
  words[14] = "\xB3\xC0\xB8\xB1";
  pans[14] = 20;
  words[15] = "\xB4\xBE\xBC";
  pans[15] = 30;
  words[16] = "\xB5\xBD\xBE\xC2";
  pans[16] = 20;
  words[17] = "\xA1\xB6";
  pans[17] = 40;
  words[18] = "\xB6\xB8\xC0\xB0\xC4";
  pans[18] = 10;
  words[19] = "\xB7\xB0\xCF\xC6";
  pans[19] = 20;


  words[20] = "";
  pans[20] = 10;
  words[21] = "\xB8\xB3\xBE\xBB\xBA\xB0";
  pans[21] = 0;
  words[22] = "\xBA\xBE\xC8\xBA\xB0";
  pans[22] = 10;
  words[23] = "\xBB\xB8\xBC\xBE\xBD";
  pans[23] = 10;
  words[24] = "\xBC\xB0\xC8\xB8\xBD\xB0";
  pans[24] = 0;
  words[25] = "\xC5\xBB\xB5\xB1";
  pans[25] = 20;
  words[26] = "\xDD\xDE\xD6\xDD\xD8\xE6\xEB";
  pans[26] = 0;
  words[27] = "\xC3\xC2\xBA\xB0";
  pans[27] = 20;
  words[28] = "\xC4\xBE\xBD\xB0\xC0\xCC";
  pans[28] = 0;
  words[29] = "\xC1\xC2\xC0\xB8\xB6\xBA\xB0";
  pans[29] = 0;

  words[30] = "";
  pans[30] = 10;
  words[31] = "\xC9\xA1\xC2\xBA\xB0";
  pans[31] = 10;
  words[32] = "\xCE\xB1\xBA\xB0";
  pans[32] = 20;
  words[33] = "\xCD\xBA\xBB\xB5\xC0";
  pans[33] = 10;
  words[34] = "\xBF\xBE\xBB\xBA\xB0";
  pans[34] = 10;
  words[35] = "\xBF\xBE\xBB\xCC\xBA\xB0";
  pans[35] = 0;
  words[36] = "\xC1\xCA\xB5\xBB";
  pans[36] = 20;
  words[37] = "\xB9\xBE\xB3\xC3\xC0\xC2";
  pans[37] = 0;
  words[38] = "\xC1\xB5\xBB";
  pans[38] = 20;
  words[39] = "\xC6\xB0\xC0\xCC";
  pans[39] = 20;

  // COLORS

  colors[0] = "\xDA\xE0\xD0\xE1\xDD\xEB\xD9";
  color_pans[0] = 10;
  colors[1] = "\xB6\xA1\xBB\xC2\xCB\xB9";
  color_pans[1] = 0;
  colors[2] = "\xD7\xD5\xDB\xf1\xDD\xEB\xD9";
  color_pans[2] = 10;
  colors[3] = "\xC1\xB8\xBD\xB8\xB9";
  color_pans[3] = 10;
  colors[4] = "\xC7\xA1\xC0\xBD\xCB\xB9";
  color_pans[4] = 0;



}

void loop() 
{
 
  // LCD


         
  // ROTARY
 
  int reading = digitalRead(ROTARY_IN);
  int color   = digitalRead(RED_DI)==LOW
                ? 1
                : digitalRead(YELLOW_DI)==LOW
                  ? 2
                  : digitalRead(GREEN_DI)==LOW
                    ? 3
                    : digitalRead(BLUE_DI)==LOW
                      ? 4
                      : digitalRead(BLACK_DI)==LOW
                        ? 5
                        : 0;

  if (!color_mode) {
    color_mode = digitalRead(RED_DI)==LOW && digitalRead(GREEN_DI)==LOW && digitalRead(BLACK_DI)==LOW;
  }

  if ((millis() - lastStateChangeTime) > FINISHED_AFTER_MS) {
  // the dial isn't being dialed, or has just finished being dialed.
    if (color_mode) {
      ask = true;
      needToPrint = false;
      operation = 7;
      color_mode = false;
      delay(3000);
    } else if (color > 0){
      needToPrint = 1;
    }

    if (needToPrint) {
    // if it's only just finished being dialed, we need to send the number down the serial
    // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.

      char charBufVar[150];
      if (operation == 7 || color > 0){
        String("\x3f").toCharArray(charBufVar, 150);
      } else {
        String(count % 10).toCharArray(charBufVar, 150);
      }

      // picture loop
      u8g.firstPage();  
      do {
        u8g.setFont(u8g_font_osr35);
        u8g.drawStr( 50, 52 , charBufVar);

        //u8g.setFont(u8g_font_osr21);
        //u8g.drawStr(pans[count % 10], 52 , charBufVar);
        
       
      } while( u8g.nextPage() );

      //delay(400);
      delay(1000);
      if (
        (operation == 1 && ((digit_1 + digit_2) == (count % 10)))
        || (operation == 2 && ((digit_1 - digit_2) == (count % 10)))
        || (operation == 3 && digit_1 == (count % 10))
        || (operation == 4 && digit_1 == (count % 10))
        || (operation == 5 && digit_1 == (count % 10))
        || (operation == 6 && digit_1 == (count % 10))
        || (operation == 7 && digit_1 == color)
      ) {
        digitalWrite(DOOR_1, HIGH);
        u8g.firstPage();  
        do {
          char charBufVar[150];
          String(count % 10).toCharArray(charBufVar, 150);
          u8g.setFont(u8g_font_osr35);
          String("\x81").toCharArray(charBufVar, 150);
          u8g.drawStr( 40, 52 , charBufVar);
        } while( u8g.nextPage() );
        delay(3000);
        digitalWrite(DOOR_1, LOW);
        operation = 0;
      }

      needToPrint = 0;
      count = 0;
      cleared = 0;
      ask = true;
    } else if (ask){
      ask = false;

      if (operation == 0) {
        operation = random(1,8);
      }
      String op = "";
      
        char charBufOperation[150];
        if (operation == 1) {
          digit_1 = random(1, 9);
          digit_2 = random(1, 10 - digit_1);
          (String(digit_1) + "+" +  String(digit_2)).toCharArray(charBufOperation, 150);
        } else if (operation == 2) {
          digit_1 = random(1, 10);
          digit_2 = random(1, digit_1);
          (String(digit_1) + "-" +  String(digit_2)).toCharArray(charBufOperation, 150);
        } else if (operation == 3) {
          digit_1 = random(0, 9);
          String(words[digit_1]).toCharArray(charBufOperation, 150);
          //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ -F
        } else if (operation == 4 || operation == 5 || operation == 6) {
          digit_1 = random(1, 9);
          String(words[digit_1 + 10 * (operation - 3)]).toCharArray(charBufOperation, 150);
          //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ -F
        } else if (operation == 7) {
            digit_1 = random(1, 6);
            String(colors[digit_1-1]).toCharArray(charBufOperation, 150);
            //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ -F
        }

      u8g.firstPage();  
      do {


        if (operation == 3) {
          u8g.setFont(u8g_font_osr21);
          u8g.drawStr(pans[digit_1], 52 , charBufOperation);
        } else if (operation == 4 || operation == 5 || operation == 6) {
          u8g.setFont(u8g_font_osr21);
          u8g.drawStr(pans[digit_1 + 10 * (operation - 3)], 52 , charBufOperation);
        } else if (operation == 7) {
            u8g.setFont(u8g_font_osr21);
            u8g.drawStr(color_pans[digit_1-1], 52 , charBufOperation);
        } else {
          u8g.setFont(u8g_font_osr35);
          u8g.drawStr(15, 52 , charBufOperation);
        }
      } while( u8g.nextPage() );
   }
  } 

  if (reading != lastState) {
    lastStateChangeTime = millis();
  }

  if ((millis() - lastStateChangeTime) > DEBOUNCE_DELAY) {
  // debounce - this happens once it's stablized
    if (reading != trueState) {
    // this means that the switch has either just gone from closed->open or vice versa.
      trueState = reading;
      if (trueState == HIGH) {
      // increment the count of pulses if it's gone high.
        u8g.firstPage();  
        do {
          u8g.drawLine(0, 10, count % 10 * 14, 10);
        } while( u8g.nextPage() );
        count++; 
        needToPrint = 1; // we'll need to print this number (once the dial has finished rotating)
      } 
    }
  }

  lastState = reading;   
}


