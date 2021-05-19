


// Time

#include <RTClib.h>
#include <Time.h>
#include <Wire.h>
#include <math.h>

// Voice
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>


// RFID
#include <deprecated.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <require_cpp11.h>

#include <SPI.h>

// LCD
#include "U8glib.h"

#define DEBUG_SERIAL        true



#define RED_DI 26
#define YELLOW_DI 25
#define GREEN_DI 24
#define BLUE_DI 23
#define BLACK_DI 22
#define SIMPLE_DI 28
#define VOICE_DI 34
#define OPEN_DI 27
#define IR_DI 35
#define PRESSURE_AI A0
#define RANDOM_AI A2

boolean voice_pressed = false;

// Time
RTC_DS3231 rtc;
#define DS3231_I2C_ADDRESS 0x68

const int MILLIS_PIN = 2; // use any digital pin you need.
 int random32khz ;  // volatile important here since we're changing this variable inside an interrupt service routine:

// Функция обработки прерывания (ISR)
void rtc_interrupt ()
{
    if(random32khz == 999)  // roll over to zero
        random32khz = 0;
    else 
        ++random32khz;

}  // end of rtc_interrupt

int clock_random(int from, int to){
  
  float interval = to - from;
   
  int result = from + trunc(interval * random32khz / 1000);
  
  DateTime now = rtc.now();
 
  random32khz = round(now.second()*16.9);

  return result;

}

//D0 D1 CS A0 RESET
U8GLIB_SSD1306_128X64 u8g(7, 6, 5, 13, 12);

// ROTARY

int reading;
int main_state = 0; // 0 reading; 1 check result
int main_result = 0;
int lastState = LOW;
int trueState = LOW;
long lastStateChangeTime = 0;
int cleared = 0;
short digit_1;
short digit_2;
short operation = 0;
String words[50];
String colors[5];
short pans[50];
short color_pans[5];
String operations[2];
boolean started = false;
boolean ask = true;
boolean simple_mode = false;
boolean sleep_mode = false;
boolean quiet_mode = false;
boolean wake_mode = false;
boolean randomize_mode = true;
boolean correct_answer = false;

#define ROTARY_IN 4 //35 to end check
#define DOOR_1 8
#define FINISHED_AFTER_MS 100
#define DEBOUNCE_DELAY 10

// RFID

unsigned long scan_rfid_time = 0;
#define RST_PIN 9
#define SS_1_PIN        33 //32 3
#define SS_2_PIN        32 //31 0
#define SS_3_PIN        31 //30 1
#define SS_4_PIN        30 //33 2




int tag_count = 0;

#define NR_OF_READERS   4
byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN};
MFRC522 mfrc522[NR_OF_READERS];

byte letter_cards[][7] = {
  {0x04, 0x63, 0xF3, 0xEA, 0xDA, 0x5F, 0x80}, // А 1
  {0x04, 0x7E, 0xE7, 0xF2, 0xDD, 0x54, 0x81}, // Б 2
  {0x34, 0x48, 0xAC, 0xA1, 0x47, 0xFE, 0xC6}, // В 3
  {0x34, 0x58, 0x34, 0x71, 0x38, 0x03, 0xC7}, // Г 4
  {0x34, 0x48, 0xAA, 0x51, 0x4A, 0xBC, 0xC6}, // Д 5
  {0x04, 0x36, 0x2F, 0xB2, 0xD5, 0x54, 0x81}, // Е 6
  {0x34, 0xC1, 0xF3, 0xB1, 0x76, 0x62, 0xA6}, // Ё 7
  {0x34, 0x50, 0x6E, 0x51, 0xAE, 0x7B, 0xC6}, // Ж 8
  {0x34, 0x50, 0x6F, 0x69, 0x4E, 0x06, 0xC6}, // З 9
  {0x34, 0x50, 0x71, 0x29, 0x10, 0x6E, 0xC6}, // И 10
  {0x34, 0x48, 0xA8, 0x29, 0x02, 0xDC, 0xC7}, // Й 11
  {0x34, 0x50, 0x6E, 0x81, 0x11, 0xAD, 0xC6}, // К 12
  {0x34, 0x50, 0x71, 0x29, 0x11, 0x1E, 0xC6}, // Л 13
  {0x34, 0xD9, 0x63, 0xA1, 0x55, 0x89, 0xA7}, // М 14
  {0x34, 0xC5, 0xDB, 0xC9, 0x1A, 0xBB, 0xA7}, // Н 15
  {0x04, 0x87, 0x8A, 0x9A, 0x5E, 0x5D, 0x81}, // О 16
  {0x34, 0x9E, 0x8D, 0x19, 0xDC, 0xA8, 0xC6}, // П 17
  {0x04, 0xCF, 0x1E, 0x5A, 0x5C, 0x5D, 0x80}, // Р 18
  {0x34, 0xC5, 0xDE, 0x41, 0xE3, 0x04, 0xA6}, // С 19
  {0x04, 0xED, 0xCD, 0x32, 0x8E, 0x5D, 0x80}, // Т 20
  {0x04, 0xC0, 0xAD, 0x1A, 0x0C, 0x61, 0x80}, // У 21
  {0x04, 0xAF, 0x15, 0xA2, 0xDD, 0x54, 0x80}, // Ф 22
  {0x34, 0xEF, 0x0D, 0x79, 0x42, 0x7D, 0x96}, // Х 23
  {0x34, 0x48, 0xAB, 0x21, 0x21, 0x77, 0xC7}, // Ц 24
  {0x04, 0xFC, 0x44, 0x6A, 0x0A, 0x61, 0x84}, // Ч 25
  {0x34, 0xD9, 0x65, 0x59, 0x45, 0x68, 0xA7}, // Щ 26
  {0x34, 0x48, 0xA8, 0x91, 0x47, 0x96, 0xC6}, // Ш 27
  {0x04, 0xDE, 0x94, 0xEA, 0xD8, 0x54, 0x80}, // Ы 28
  {0x34, 0x58, 0x37, 0x41, 0xEF, 0x7C, 0xC6}, // Э 29
  {0x34, 0xD9, 0x63, 0xA1, 0x38, 0xA1, 0xA7}, // Ю 30
  {0x34, 0xD9, 0x68, 0x69, 0x1C, 0x04, 0xA7}  // Я 31
};

byte digit_cards[][7] = {
  {0x04, 0x3D, 0x29, 0xAA, 0x0B, 0x61, 0x85}, // 1
  {0x04, 0x10, 0x3F, 0xAA, 0xDD, 0x54, 0x85}, // 2
  {0x04, 0x49, 0x6D, 0xE2, 0xD8, 0x54, 0x80}, // 3
  {0x04, 0xE7, 0x32, 0xC2, 0xD8, 0x54, 0x84}, // 4
  {0x04, 0x25, 0x10, 0xDA, 0xDD, 0x54, 0x85}, // 5
  {0x34, 0xC1, 0xF3, 0x81, 0xE7, 0x81, 0xA6}, // 6
  {0x04, 0x80, 0xBA, 0x0A, 0x8E, 0x5D, 0x81}, // 7
  {0x34, 0x50, 0x71, 0x49, 0x51, 0x3A, 0xC7}, // 8
  {0x04, 0x72, 0xB0, 0xC2, 0xDD, 0x54, 0x80}, // 9
  {0x34, 0x48, 0xAB, 0xB9, 0x10, 0x88, 0xC6} // 0
};

int word_cards[][4] = {
  {5, 1, 0, 0}, // ДА
  {4, 1, 5, 0}, // ГАД
};

// Voice

SoftwareSerial voice_serial(10, 36); // RX, TX
DFPlayerMini_Fast voice_mp3;

// Main

#define NUMBER_OF_FUNCTIONS 15

void (*get_functions[NUMBER_OF_FUNCTIONS])();
void (*show_result_functions[NUMBER_OF_FUNCTIONS])();
boolean (*check_result_functions[NUMBER_OF_FUNCTIONS])();
void (*get_question_functions[NUMBER_OF_FUNCTIONS])();

void debug(String message) {
  if (DEBUG_SERIAL) {
    Serial.println(message);
  }
}

void debug(int message) {
  if (DEBUG_SERIAL) {
    Serial.println(message);
  }
}

void debug(boolean message) {
  if (DEBUG_SERIAL) {
    Serial.println(message);
  }
}

void setup() {

  debug("setup");

//  randomSeed(analogRead(RANDOM_AI));

  get_question_functions[1] = get_question_1;
  get_functions[1] = get_rotary;
  show_result_functions[1] = show_result_digit;
  check_result_functions[1] = check_result_1;

  get_question_functions[2] = get_question_2;
  get_functions[2] = get_rotary;
  show_result_functions[2] = show_result_digit;
  check_result_functions[2] = check_result_2;

  get_question_functions[3] = get_question_3;
  get_functions[3] = get_rotary;
  show_result_functions[3] = show_result_digit;
  check_result_functions[3] = check_result_3;

  get_question_functions[4] = get_question_4;
  get_functions[4] = get_rotary;
  show_result_functions[4] = show_result_digit;
  check_result_functions[4] = check_result_4;

  get_question_functions[5] = get_question_5;
  get_functions[5] = get_rotary;
  show_result_functions[5] = show_result_digit;
  check_result_functions[5] = check_result_equal;

  get_question_functions[6] = get_question_equal;
  get_functions[6] = get_rotary;
  show_result_functions[6] = show_result_digit;
  check_result_functions[6] = check_result_equal;

  get_question_functions[7] = get_question_equal;
  get_functions[7] = get_rotary;
  show_result_functions[7] = show_result_digit;
  check_result_functions[7] = check_result_equal;

  get_question_functions[8] = get_question_equal;
  get_functions[8] = get_rotary;
  show_result_functions[8] = show_result_slash;
  check_result_functions[8] = check_result_equal;

  get_question_functions[9] = get_question_equal;
  get_functions[9] = get_rotary;
  show_result_functions[9] = show_result_digit;
  check_result_functions[9] = check_result_equal;

  get_question_functions[10] = get_question_color;
  get_functions[10] = get_color;
  show_result_functions[10] = show_result_slash;
  check_result_functions[10] = check_result_color;

  get_question_functions[11] = get_question_rfid_letter;
  get_functions[11] = get_rfid_letter;
  show_result_functions[11] = show_result_dummy;
  check_result_functions[11] = check_result_rfid;

  get_question_functions[12] = get_question_rfid_digit;
  get_functions[12] = get_rfid_digit;
  show_result_functions[12] = show_result_dummy;
  check_result_functions[12] = check_result_rfid;

  get_question_functions[13] = get_question_digit;
  get_functions[13] = get_rotary_digit;
  show_result_functions[13] = show_result_digit;
  check_result_functions[13] = check_result_equal;

  get_question_functions[14] = get_question_rfid_word;
  get_functions[14] = get_rfid_word;
  show_result_functions[14] = show_result_dummy;
  check_result_functions[14] = check_result_rfid;

  pinMode(SIMPLE_DI,INPUT);
  digitalWrite(SIMPLE_DI,HIGH);

  pinMode(VOICE_DI,INPUT);
  digitalWrite(VOICE_DI,HIGH);

  pinMode(OPEN_DI,INPUT);
  digitalWrite(OPEN_DI,HIGH);

  pinMode(IR_DI,INPUT);
  digitalWrite(IR_DI,HIGH);

  pinMode(PRESSURE_AI,INPUT);

  

  if (DEBUG_SERIAL) {
    Serial.begin(115200);           // Initialize serial communications with the PC
    while (!Serial);              // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  }

  // Time


  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(2021,05,20,01,01,34));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  //rtc.adjust(DateTime(2020,11,23,23,58,00));



//  Wire.beginTransmission(DS3231_I2C_ADDRESS);
//  Wire.write(0x0E);
//  Wire.write(B00000000);
//  Wire.endTransmission();

//    pinMode(MILLIS_PIN, INPUT);
    // Global Enable INT0 interrupt
//    EIMSK |= ( 1 << INT0);
    // Signal change triggers interrupt
//    EICRA |= ( 1 << ISC00);
//    EICRA |= ( 0 << ISC01);
  // set up to handle interrupt from 1 Hz pin
 DateTime now = rtc.now();
 
  random32khz = round(now.second()*16.9);
  debug(random32khz);
  pinMode (MILLIS_PIN, INPUT);
  attachInterrupt (digitalPinToInterrupt (MILLIS_PIN), rtc_interrupt, CHANGE);

  // Time
  if (DEBUG_SERIAL ) {
    digitalClockDisplay();
  }



  // RFID
  scan_rfid_time = millis();

  SPI.begin();                  // Init SPI bus

  /* looking for MFRC522 readers */
  //https://forum.arduino.cc/t/extend-range-for-mfrc522-rfid-reader/194878/3
  //byte RFCfgReg = 0x26 << 1; // configures the receiver gain
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    mfrc522[reader].PCD_AntennaOff();
    mfrc522[reader].PCD_SetAntennaGain(0x07<<4);
    mfrc522[reader].PCD_WriteRegister(mfrc522[reader].RFCfgReg, (0x07<<4)); // Set Rx Gain to max

//    mfrc522[reader].PCD_AntennaOff();
//    mfrc522[reader].PCD_AntennaOn();
  
    if (DEBUG_SERIAL) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": AntennaGain = "));
    mfrc522[reader].PCD_SetAntennaGain(0x07<<4);
      Serial.println(mfrc522[reader].PCD_GetAntennaGain());
   //   mfrc522[reader].PCD_DumpVersionToSerial();
    }

    
  }


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

  // DOOR "\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ -F
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

  words[40] = "";
  pans[40] = 0;
  words[41] = "\xDF\xDE\xDD\xD5\xD4\xD5\xDB\xEC\xDD\xD8\xDA";
  pans[41] = 0;
  words[42] = "\xD2\xE2\xDE\xE0\xDD\xD8\xDA";
  pans[42] = 10;
  words[43] = "\xE1\xE0\xD5\xD4\xD0";
  pans[43] = 20;
  words[44] = "\xE7\xD5\xE2\xD2\xD5\xE0\xD3";
  pans[44] = 10;
  words[45] = "\xDF\xEF\xE2\xDD\xD8\xE6\xD0";
  pans[45] = 10;
  words[46] = "\xE1\xE3\xD1\xD1\xDE\xE2\xD0";
  pans[46] = 10;
  words[47] = "\xD2\xDE\xE1\xDA\xE0\xD5\xE1\xD5\xDD\xEC\xD5";
  pans[47] = 0;
  words[48] = "";
  pans[48] = 20;
  words[49] = "";
  pans[49] = 20;

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

  // OPERATIONS

  operations[0] = "\xBF\xBB\xCE\xC1";
//  operations_pans[0] = 20;
  operations[1] = "\xBC\xB8\xBD\xC3\xC1";
//  operations_pans[1] = 10;


// Voice

  voice_serial.begin(9600);

  voice_mp3.begin(voice_serial);
  
  debug("Setting volume to 60%");
  voice_mp3.volume(16); //16

  wake_mode = false;
  started = false;
  ask = true;
  simple_mode = false;
  sleep_mode = false;
  quiet_mode = false;
  wake_mode = false;
  randomize_mode = true;
  correct_answer = false;

}

void loop() 
{ 
   // Rotary
  reading = digitalRead(ROTARY_IN);

         
  // Inputs
 
  simple_mode = digitalRead(SIMPLE_DI)==LOW;


  //Serial.println(digitalRead(IR_DI));

  

  if (digitalRead(OPEN_DI)==LOW) { // cower is open

    if ((millis() - lastStateChangeTime) > FINISHED_AFTER_MS){
  
      if (sleep_mode ) {
        if (!wake_mode) {
          // clear lcd 
          u8g.firstPage(); 
          do {
           // do nothing
          } while( u8g.nextPage() );
          wake_mode = true;
        }
      } else if (wake_mode) {
        ask = true;
        quiet_mode = true;
        wake_mode = false;
  
        DateTime now = rtc.now();
  
        if (operation != 0 && now.hour() >= 10 && now.hour() < 22 && clock_random(1,13)==3){
            if (clock_random(0,2) == 1) {
              voice_mp3.wakeUp();
              voice_mp3.play(clock_random(0,2) == 1 ? 51 : 32);
            
              delay(2000);
            
              voice_mp3.sleep();
            } else {
              if (operation != 0) {
                voice_pressed = true;
                get_functions[operation]();
              }
            }
        } 
      }
      if (!sleep_mode) {
      // the dial isn't being dialed, or has just finished being dialed.
  
  
        if (analogRead(PRESSURE_AI) > 150) {
            digitalWrite(DOOR_1, HIGH);
            delay(1000);
            digitalWrite(DOOR_1, LOW);
        }
    
        if (main_state == 1) {
        // if it's only just finished being dialed, we need to send the number down the serial
        // line and reset the main_result. We mod the main_result by 10 because '0' will send 10 pulses.
    
          show_result_functions[operation]();
    
          //delay(400);
          delay(1000);
  
          correct_answer = check_result_functions[operation]();
          if (correct_answer) {
  
            
            voice_mp3.wakeUp();
            voice_mp3.play(51);
          
            delay(1000);
          
            voice_mp3.sleep();
  
  
            
            digitalWrite(DOOR_1, HIGH);
            u8g.firstPage();  
            do {
              char charBufVar[150];
              String(main_result % 10).toCharArray(charBufVar, 150);
              u8g.setFont(u8g_font_osr35);
              String("\x81").toCharArray(charBufVar, 150); // q
              u8g.drawStr( 40, 52 , charBufVar);
            } while( u8g.nextPage() );
            delay(6000);
            digitalWrite(DOOR_1, LOW);
            operation = 0;
            randomize_mode = true;
          }  else {
            voice_mp3.wakeUp();
            voice_mp3.play(52);
          
            delay(3000);
          
            voice_mp3.sleep();
            randomize_mode = !simple_mode;
          }
    
          main_state = 0;
          main_result = 0;
          cleared = 0;
          ask = true;
        } else if (ask){
          ask = false;
    
          if (operation == 0) {//clock_random(10,14)
            operation = simple_mode ? clock_random(11,15) : clock_random(1,11);
          }
  
          debug( "get_question_functions:" );
          debug( operation );
  
          get_question_functions[operation]();
          quiet_mode = false;
  
        }
      } 
  
  
      if (operation != 0 && !sleep_mode) {
        get_functions[operation]();
      }
      sleep_mode = digitalRead(IR_DI)==LOW;
    }
  } else {
    debug(analogRead(PRESSURE_AI));
  }

}

void digitalClockDisplay(){
  DateTime now = rtc.now();
  // digital clock display of the time
  Serial.print(now.hour());
  printDigits(now.minute());
  printDigits(now.second());
  Serial.print(" ");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.month());
  Serial.print(" ");
  Serial.print(now.year()); 
  Serial.println(); 
}
 
void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void get_dummy() {
}
void show_result_dummy() {
}

void get_question_1(){
  String op = "";
      
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];

  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);


  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, 9);
    digit_2 = clock_random(1, 10 - digit_1);
  }
  (String(digit_1) + "+" +  String(digit_2)).toCharArray(charBufOperation2, 150);

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr35);
    u8g.drawStr(15, 52 , charBufOperation2);
  } while( u8g.nextPage() );
}

void get_question_2(){
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, 10);
    digit_2 = clock_random(1, digit_1);
  }
  (String(digit_1) + "-" +  String(digit_2)).toCharArray(charBufOperation2, 150);
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr35);
    u8g.drawStr(15, 52 , charBufOperation2);
  } while( u8g.nextPage() );
}

void get_question_3(){
  debug("get_question_3");
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, 9);
    digit_2 = clock_random(1, 10 - digit_1);
  }
  (words[digit_1]).toCharArray(charBufOperation1, 150);
  (operations[0]).toCharArray(charBufOperation2, 150);
  (words[digit_2]).toCharArray(charBufOperation3, 150);
  
  u8g.firstPage();  
  do {
    debug("u8g");
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(10, 18 , charBufOperation1);
    u8g.drawStr(10, 40 , charBufOperation2);
    u8g.drawStr(10, 62 , charBufOperation3);
  } while( u8g.nextPage() );
}

void get_question_4(){
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, 10);
    digit_2 = clock_random(1, digit_1);
  }
  (words[digit_1]).toCharArray(charBufOperation1, 150);
  (operations[1]).toCharArray(charBufOperation2, 150);
  (words[digit_2]).toCharArray(charBufOperation3, 150);
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(10, 18 , charBufOperation1);
    u8g.drawStr(10, 40 , charBufOperation2);
    u8g.drawStr(10, 62 , charBufOperation3);
   } while( u8g.nextPage() );
}

void get_question_5(){
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(0, 9);
  }
  String(words[digit_1]).toCharArray(charBufOperation2, 150);
  //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ - F

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(10, 18 , charBufOperation1);
    u8g.drawStr(10, 40 , charBufOperation2);
    u8g.drawStr(10, 62 , charBufOperation3);
  } while( u8g.nextPage() );
}

void get_question_equal(){
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, operation == 9 ? 8 : 10);
  }
  String(words[digit_1 + 10 * (operation - 5)]).toCharArray(charBufOperation2, 150);
  //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ - F
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(pans[digit_1 + 10 * (operation - 5)], 52 , charBufOperation2);
  } while( u8g.nextPage() );
}


void get_rotary () {
    if (reading != lastState) {
      lastStateChangeTime = millis();
    }
  
    if ((millis() - lastStateChangeTime) > DEBOUNCE_DELAY) {
    // debounce - this happens once it's stablized
      if (reading != trueState) {
      // this means that the switch has either just gone from closed->open or vice versa.
        trueState = reading;
        if (trueState == HIGH) {
        // increment the main_result of pulses if it's gone high.
          u8g.firstPage();  
          do {
            u8g.drawLine(0, 10, main_result % 10 * 14, 10);
          } while( u8g.nextPage() );
          main_result++; 
          main_state = 1; // we'll need to print this number (once the dial has finished rotating)
        } 
      }
    }
    lastState = reading;
}

// rotary

void show_result_digit(){
  char charBufVar[150];
  String(main_result % 10).toCharArray(charBufVar, 150);

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr35);
    u8g.drawStr( 50, 52 , charBufVar);

    //u8g.setFont(u8g_font_osr21);
    //u8g.drawStr(pans[main_result % 10], 52 , charBufVar);
    
   
  } while( u8g.nextPage() );
}

boolean check_result_1(){
  return digit_1 + digit_2 == (main_result % 10);
}
  
boolean check_result_2(){
  return  digit_1 - digit_2 == (main_result % 10);
}

boolean check_result_3(){
  return digit_1 + digit_2 == (main_result % 10);
}

boolean check_result_4(){
  return digit_1 - digit_2 == (main_result % 10);
}

boolean check_result_equal(){
  return digit_1 == (main_result % 10);
}


// color

void get_question_color(){
  if (!quiet_mode) {
    voice_mp3.wakeUp();
    voice_mp3.play(32);
  
    delay(2000);
    voice_mp3.sleep();
  }

  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  debug("digit_1");
  debug(digit_1);
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, 6);
  }
  debug(digit_1);
  String(colors[digit_1-1]).toCharArray(charBufOperation2, 150);
  //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ - F

  u8g.firstPage();  

  do {
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(color_pans[digit_1-1], 52 , charBufOperation2);
  } while( u8g.nextPage() );
}

void get_color (){
  check_voice();
  if (voice_pressed) {
    debug("voice pressed");

    voice_mp3.wakeUp();
  
    voice_mp3.play(42 + digit_1);

    debug(42 + digit_1);
    delay(2000);
    voice_mp3.sleep();
    voice_pressed = false;
  } 

  
  main_result = digitalRead(RED_DI)==LOW
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
  if (main_result != 0) {
    main_state = 1;
  }
}

void show_result_slash() {// 8, 10
  char charBufVar[150];
  String("\x3f").toCharArray(charBufVar, 150); // /

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr35);
    u8g.drawStr( 50, 52 , charBufVar);

    //u8g.setFont(u8g_font_osr21);
    //u8g.drawStr(pans[main_result % 10], 52 , charBufVar);
    
   
  } while( u8g.nextPage() );
}

boolean check_result_color(){
  return  digit_1 == main_result;
}



// RFID
/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void get_question_rfid_letter(){
//  get_question_rfid(32);
  get_question_rfid(10);
}

void get_rfid_letter() {
        check_card(letter_cards, 0, 0);
}

void get_question_rfid_digit(){
  get_question_rfid(10);
//  get_question_rfid(5);
}

void get_question_rfid_word(){
  get_question_rfid(2);
//  get_question_rfid(5);
}

void get_rfid_digit() {
        check_card(digit_cards, 32, 50);
}

void get_rfid_word() {
        check_cards(letter_cards, word_cards[digit_1-1], 53, 53);
}

boolean check_result_rfid(){
  return main_result == 1;
}


void check_voice (){
  if (!voice_pressed) {
    voice_pressed = digitalRead(VOICE_DI)==LOW;
  }
}

void get_question_rfid(int size){

  if (!quiet_mode) {
    voice_mp3.wakeUp();
    voice_mp3.play(32);
  
    delay(2000);
    voice_mp3.sleep();
  }

  
  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  
  if (!quiet_mode && randomize_mode) {
    digit_1 = clock_random(1, size+1);
    debug("get digit");
    debug(digit_1);
  }
  String("\x3f").toCharArray(charBufOperation2, 150); // ? "\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ - F
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr35);
    u8g.drawStr( 40, 52 , charBufOperation2);
  } while( u8g.nextPage() );
}


bool rfid_tag_present_prev[4] = {false,false,false,false};
bool rfid_tag_present[4] = {false,false,false,false};
int _rfid_error_counter[4] = {0,0,0,0};
bool _tag_found[4] = {false,false,false,false};
byte present_cards[4][7]  = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


boolean scan_card(int reader){ //https://github.com/miguelbalboa/rfid/issues/352#issue-282870788
  boolean newFound = false;
  delay(100);

//    mfrc522[reader].PCD_SetAntennaGain(0x07<<4);
  mfrc522[reader].PCD_AntennaOn();

  rfid_tag_present_prev[reader] = rfid_tag_present[reader];

  _rfid_error_counter[reader] += 1;
  if(_rfid_error_counter[reader] > 2){
    _tag_found[reader] = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].TxModeReg, 0x00);
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].RxModeReg, 0x00);
  // Reset ModWidthReg
  mfrc522[reader].PCD_WriteRegister(mfrc522[reader].ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfrc522[reader].PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfrc522[reader].STATUS_OK){
    if ( ! mfrc522[reader].PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
        mfrc522[reader].PCD_AntennaOff();
        return newFound;
    }
    _rfid_error_counter[reader] = 0;
    _tag_found[reader] = true;        
  }
  
  rfid_tag_present[reader] = _tag_found[reader];
  
  // rising edge
  if (rfid_tag_present[reader] && !rfid_tag_present_prev[reader]){
    Serial.println("Tag found");
    for (int i = 0; i < mfrc522[reader].uid.size; i++) {
      present_cards[reader][i] = mfrc522[reader].uid.uidByte[i];
    }
    newFound = true;
  }
  
  // falling edge
  if (!rfid_tag_present[reader] && rfid_tag_present_prev[reader]){
    Serial.println("Tag gone");
    for (int i = 0; i < 7; i++) {
      present_cards[reader][i] = 0x00;
    }

  }

  mfrc522[reader].PCD_AntennaOff();
  return newFound;
}

int scan_cards(){
  int reader_new_card = 0;
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    check_voice(); 
    if (scan_card(reader)) {
      reader_new_card = reader +1;
    }
  }

  return reader_new_card;
}

void check_card(byte cards[][7], int voice_shift, int voice_type) {
        check_voice();
        if (voice_pressed) {
          debug("voice pressed");

          voice_mp3.wakeUp();
        

          if (voice_type != 0) {
            voice_mp3.play(voice_type);
            delay(1700);
          }
          voice_mp3.play(voice_shift + digit_1);
          debug(voice_shift + digit_1);
          delay(2000);
          voice_mp3.sleep();
          voice_pressed = false;
        } 
        check_voice(); 

        if (scan_rfid_time > millis() || millis()-scan_rfid_time > 1000) { // Check rfid only one time in sec
          scan_rfid_time = millis();

          int reader_new_card = scan_cards();
          for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
            dump_byte_array(present_cards[reader], 7);
            Serial.println();
          }
          check_voice();                          

          if (reader_new_card > 0) {
              for (int i = 0; i < 7; i++) {       //tagarray's columns
                check_voice();
                if ( present_cards[reader_new_card-1][i] != cards[digit_1-1][i]) {  //Comparing the UID in the buffer to the UID in the tag array.
                    main_state = 1;
                    main_result = 0;
                  return;
                } else {
                  if (i == 6) {                // Test if we browesed the whole UID.
                    main_state = 1;
                    main_result = 1;
                    return;
                  } else {
                    continue;                                           // We still didn't reach the last cell/column : continue testing!
                  }
                }
              }
          }
        }
        main_state = 0;

}

void check_cards(byte cards[][7], int card_numbers[], int voice_shift, int voice_type) {
        check_voice();
        if (voice_pressed) {
          debug("voice pressed");

          voice_mp3.wakeUp();
        

          if (voice_type != 0) {
            voice_mp3.play(voice_type);
            delay(1700);
          }
          voice_mp3.play(voice_shift + digit_1);
          debug(voice_shift + digit_1);
          delay(2000);
          voice_mp3.sleep();
          voice_pressed = false;
        } 
        check_voice(); 


        int letter_count = 0;
        for (int i = 0; i < 4; i++){
          if (card_numbers[i] != 0) {
            letter_count++;
          }
        }
 
        if (scan_rfid_time > millis() || millis()-scan_rfid_time > 1000) { // Check rfid only one time in sec
          scan_rfid_time = millis();
            // RFID Loop
          int reader_new_card = scan_cards();

          int card_count = 0;
          boolean has_new_card = false;
          for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
              boolean isCard = false;
              for (int i = 0; i < 7; i++) {       //tagarray's columns
                if (present_cards[reader][i] != 0x00){
                  isCard = true;
                }
              }
              if (isCard) {
                card_count++;
              }
          }

          if (reader_new_card && card_count == letter_count) {
            int current_card = 0;
            for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
                    for (int i = 0; i < 7; i++) {       //tagarray's columns
                      if ( present_cards[reader][i] != cards[card_numbers[current_card]-1][i]) {  //Comparing the UID in the buffer to the UID in the tag array.
                        main_state = 1;
                        main_result = 0;
                        return;
                      } else {
                       if (i == 6 && current_card == letter_count-1) {                // Test if we browesed the whole UID.
                          main_state = 1;
                          main_result = 1;
                          return;
                        } else {
                          continue;                                           // We still didn't reach the last cell/column : continue testing!
                        }
                      }
                    }
      
                    check_voice();
                    current_card++;
            }
          }
        }
        main_state = 0;

}

// Rotary Audio Digit

void get_question_digit(){
  if (!quiet_mode) {

    voice_mp3.wakeUp();
    voice_mp3.play(32);
  
    delay(2000);
  
    voice_mp3.sleep();
  }

  String op = "";
  
  char charBufOperation1[150];
  char charBufOperation2[150];
  char charBufOperation3[150];
  
  op.toCharArray(charBufOperation1, 150);
  op.toCharArray(charBufOperation2, 150);
  op.toCharArray(charBufOperation3, 150);
  
  if (!quiet_mode && randomize_mode) {
//    digit_1 = clock_random(0, 10);
    digit_1 = clock_random(1, 10);
    debug("get digit");
    debug(digit_1);
  }
  String(words[digit_1]).toCharArray(charBufOperation2, 150);
  //"\xdf\xdb"; http://www.codenet.ru/services/urlencode-urldecode/ - F

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_osr21);
    u8g.drawStr(10, 18 , charBufOperation1);
    u8g.drawStr(10, 40 , charBufOperation2);
    u8g.drawStr(10, 62 , charBufOperation3);
  } while( u8g.nextPage() );
}

void get_rotary_digit () {
    check_voice();
    if (voice_pressed) {
      debug("voice pressed");

      voice_mp3.wakeUp();
      voice_mp3.play(49); // Набери

      delay(1400);
    
      voice_mp3.play(32 + (digit_1 == 0 ? 10 : digit_1));

      debug(32 + (digit_1 == 0 ? 10 : digit_1));
      delay(2000);
      voice_mp3.sleep();
      voice_pressed = false;
    } 
    check_voice(); 

    get_rotary();
}

// Количество белых
// Категории (Овощи, Фрукты, Живые, Не живые)
