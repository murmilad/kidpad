// LCD
#include "U8glib.h"


/* Digital pin number for the displays chip select pin */
#define CS_DI 10
/* Digital pin number for the displays data/command pin */
#define DC_DI 9
/* Digital pin number for the displays reset pin */
#define RST_DI 8


U8GLIB_SSD1306_128X64 u8g(7, 6, 5, 3, 2);

// ROTARY

int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long lastStateChangeTime = 0;
int cleared = 0;

#define ROTARY_IN 4
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
        u8g.drawStr( 50, 60 , charBufVar);
      } while( u8g.nextPage() );

      needToPrint = 0;
      count = 0;
      cleared = 0;
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



