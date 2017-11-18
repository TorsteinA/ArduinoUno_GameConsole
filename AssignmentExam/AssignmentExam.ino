#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <Wire.h> // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include "pitches.h"

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
const int TFT_CS   = 10;
const int TFT_RST  = 8;
const int TFT_DC   = 9;
const int TFT_SCLK = 13;
const int TFT_MOSI = 11;
const int analogInPinY = A0;
const int analogInPinX = A1;
const int buttonPin = 2;
//const int tonePin = 2;

int sensorValueY = 0;
int sensorValueX = 0;
int buttonState = 0;          // current state of the button
int buttonState2 = 0;         // current state of the button2
int lastButtonState = 0;      // previous state of the button


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
RTC_DS1307 rtc;

DateTime now;

int melodyAlarm[] = {
  //NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A4, NOTE_B4, NOTE_C4, NOTE_B4, NOTE_A4, NOTE_G3, NOTE_F3, NOTE_E3, NOTE_D3
  NOTE_E3,
  NOTE_GS3,
  NOTE_B3,
  NOTE_DS3,
  NOTE_E4,
  NOTE_GS4,
  NOTE_B4,
  NOTE_DS4,
  NOTE_E5,
  NOTE_DS4,
  NOTE_B4,
  NOTE_GS4,
  NOTE_E4,
  NOTE_B3,
  NOTE_GS3/*,
  NOTE_E3*/
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurationsAlarm[] = {
  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, //8, 4, 1
};

int noteDurationsRandomAlarm[] = {
  16, 16, 16, 16, 8, 8, 4, 2
};

int melodyStartup[] = {
  NOTE_C3, NOTE_D3, NOTE_E3, NOTE_F3, NOTE_G3, NOTE_A4, NOTE_B4, NOTE_C4//, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B5, NOTE_C5
};

int noteDurationsStartup[] = {
  1, 2, 4, 8, 16, 32, 32 
};

void setup(void) {

  pinMode(buttonPin, INPUT);
  pinMode(analogInPinY, INPUT);
  pinMode(analogInPinX, INPUT);
  randomSeed(analogRead(0));

  while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(57600);


  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(3); // Flips the output upside down. This is due to how I placed the hardware

  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  runStartUpAnimation(ST7735_RED, ST7735_BLUE);
  runStartUpAnimation(ST7735_BLACK, ST7735_YELLOW);
  tft.fillScreen(ST7735_BLACK);
  
  
  //PlayStartupAudio();
  //PlayRandomStartupAudio();

  
  Serial.println("Initialized");
}

void loop() {
  now = rtc.now();
  buttonState = digitalRead(buttonPin);
  sensorValueY = analogRead(analogInPinY);
  sensorValueX = analogRead(analogInPinX);

  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      pressedJoystickButton();
    } else {
      releasedJoystickButton();
    }
    delay(100);
    lastButtonState = buttonState;
  }

}

void pressedJoystickButton(){
  Serial.println("Pressed Button! :D");
  showTextLine(0, "Button is being pressed", ST7735_GREEN);
}

void releasedJoystickButton(){
  hideTextLine(0);
}



void showTextLine(int yValue, String text, uint16_t color) {
  showText(0, yValue, text, color);
}

void showText(int xValue, int yValue, String text, uint16_t color) {
  tft.setCursor(xValue, yValue);
  tft.setTextColor(color);
  tft.println(text);
}

void hideText(int xStart, int yStart, int width) {
  tft.fillRect(xStart, yStart, width, 10 /*default font size*/, ST7735_BLACK);
}

void hideTextLine(int yValue) {
  tft.fillRect(0, yValue, tft.width(), 10 /*default font size*/, ST7735_BLACK);
}

// --- Animations --- //

void runStartUpAnimation(uint16_t color1, uint16_t color2) {
  //tft.fillScreen(ST7735_BLACK);
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color1);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, color2);
  }
  //Clear Screen
}


// --- Audio Methods --- //
/*
void PlayAlarmAudio() {
  for (int i = 0; i <= (sizeof(melodyAlarm) / sizeof(melodyAlarm[0])) - 1; i++) {
    int noteDuration = 1000 / noteDurationsAlarm[i % (sizeof(noteDurationsAlarm) / sizeof(noteDurationsAlarm[0]))];
    tone(tonePin, melodyAlarm[i % (sizeof(melodyAlarm) / sizeof(melodyAlarm[0]))], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(tonePin);
  }
}

void PlayStartupAudio() {
  for (int i = 0; i <= (sizeof(melodyStartup) / sizeof(melodyStartup[0])) - 1; i++) {
    int noteDuration = 1000 / noteDurationsStartup[i % (sizeof(noteDurationsStartup) / sizeof(noteDurationsStartup[0]))];
    tone(tonePin, melodyStartup[i % (sizeof(melodyStartup) / sizeof(melodyStartup[0]))], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(tonePin);
  }
}

void PlayRandomAlarmAudio(){
  double melodyLength = (sizeof(melodyStartup) / sizeof(melodyStartup[0]));
  for (int i = 0; i <= melodyLength - 1; i++) {
    int noteDuration = 1000 / noteDurationsRandomAlarm[random(sizeof(noteDurationsRandomAlarm) / sizeof(noteDurationsRandomAlarm[0]))];
    tone(tonePin, melodyAlarm[random(melodyLength)], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(tonePin);
  }
}

void PlayRandomStartupAudio(){
  double melodyLength = (sizeof(melodyStartup) / sizeof(melodyStartup[0]));
  for (int i = 0; i <= melodyLength - 1; i++) {
    int noteDuration = 1000 / noteDurationsStartup[i % (sizeof(noteDurationsStartup) / sizeof(noteDurationsStartup[0]))];
    tone(tonePin, melodyStartup[random(melodyLength)], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(tonePin);
  }*/

