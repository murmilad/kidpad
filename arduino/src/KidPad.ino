// LCD
#include "U8glib.h"


/* Digital pin number for the displays chip select pin */
#define CS_DI 10
/* Digital pin number for the displays data/command pin */
#define DC_DI 9
/* Digital pin number for the displays -reset pin */
#define RST_DI 8

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
  
  // ROTARY
  
  pinMode(ROTARY_IN, INPUT);

  // DOOR
  randomSeed(analogRead(0));
  pinMode(DOOR_1, OUTPUT);
 
}

void loop() 
{
 
  // LCD


         
  // ROTARY
 
  int reading = digitalRead(ROTARY_IN);

  if ((millis() - lastStateChangeTime) > FINISHED_AFTER_MS) {
  // the dial isn't being dialed, or has just finished being dialed.
    if (needToPrint) {
    // if it's only just finished being dialed, we need to send the number down the serial
    // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
  
      // picture loop
      u8g.firstPage();  
      do {
        char charBufVar[50];
        String(count % 10).toCharArray(charBufVar, 50);

        // graphic commands to redraw the complete screen should be placed here  
        u8g.setFont(u8g_font_osr35);
        //u8g.setFont(u8g_font_osb21);
        u8g.drawStr( 50, 52 , charBufVar);
        
       
      } while( u8g.nextPage() );

      delay(400);
      if (
        (operation == 1 && ((digit_1 + digit_2) == (count % 10)))
        || (operation == 2 && ((digit_1 - digit_2) == (count % 10)))
      ) {
        digitalWrite(DOOR_1, HIGH);
        u8g.firstPage();  
        do {
          char charBufVar[150];
          String(count % 10).toCharArray(charBufVar, 150);
          String("Open").toCharArray(charBufVar, 150);
          u8g.drawStr( 10, 52 , charBufVar);
        } while( u8g.nextPage() );
        delay(3000);
        digitalWrite(DOOR_1, LOW);
      }

      operation = 0;
      needToPrint = 0;
      count = 0;
      cleared = 0;
    } else if (operation == 0) {
      operation = random(1,3);
      String op = "";
      if (operation == 1) {
        digit_1 = random(9);
        digit_2 = random(9 - digit_1);
        op = "+";
      } else if (operation == 2) {
        digit_1 = random(9);
        digit_2 = random(digit_1);
        op = "-";
      }
      
      
      u8g.firstPage();  
      do {
        char charBufOperation[150];
        (String(digit_1) + op +  String(digit_2)).toCharArray(charBufOperation, 150);
        // graphic commands to redraw the complete screen should be placed here  
         u8g.setFont(u8g_font_osr35);
        //u8g.setFont(u8g_font_osb21);
        u8g.drawStr( 15, 52 , charBufOperation);
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


