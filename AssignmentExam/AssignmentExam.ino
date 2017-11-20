#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <Wire.h> // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
//#include "pitches.h"
#include "gameOneStone.h"

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

int sensorValueY = 0;
int sensorValueX = 0;
int buttonState = 0;          // current state of the button
int buttonState2 = 0;         // current state of the button2
int lastButtonState = 0;      // previous state of the button


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
const uint16_t backgroundColor = ST7735_BLACK;

RTC_DS1307 rtc;

DateTime now, previousTime;

int state = 0;  // 0 = main Menu, 1 = game1, 2 = game2, 3 = game3 (2 and 3 goes to not-implemented);
int previousState;

// Main Menu
int menuSelection = 0;
int numberOfMenuSelections = 3;

// Game One
int playerX, previousPlayerX, playerY, previousPlayerY;
const int maxSpeed = 10;
int scoreTimer = 0;
int stoneStartSpeed = 2;
int numberOfStones = 0;

gameOneStone* gos = NULL;



void setup(void) {

  pinMode(buttonPin, INPUT);
  pinMode(analogInPinY, INPUT);
  pinMode(analogInPinX, INPUT);
  randomSeed(analogRead(0));

  while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(57600);


  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(2); // Flips the output upside down. This is due to how I placed the hardware

  
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
  previousTime = now;

  runStartUpAnimation(ST7735_BLACK, ST7735_YELLOW);
  tft.fillScreen(ST7735_BLACK);
  
  showMainMenu();
  
  Serial.println("Initialized");
}

void loop() {
  now = rtc.now();
  buttonState = digitalRead(buttonPin);
  sensorValueY = analogRead(analogInPinY);
  sensorValueX = analogRead(analogInPinX);

  // Check input
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      pressedJoystickButton();
    } else {
      releasedJoystickButton();
    }
    delay(100);
    lastButtonState = buttonState;
  }

  // Update state
  if (state != previousState){
    tft.fillScreen(ST7735_BLACK);
    if (state == 0){
      showMainMenu();
    } else if (state == 1){
      startGameOne();
    } else {
      showNotImplemented();
    }
    previousState = state;
  }

  // Display
  if (state == 0){
    // Main Menu
    mainMenu();

  } else if (state == 1){
    // Game One
    

    // Logic
    previousPlayerX = playerX;
    previousPlayerY = playerY;

    playerX += map(sensorValueX, 30, 1000, -maxSpeed, maxSpeed);
    if (sensorValueX > 450 && sensorValueX < 550) playerX = previousPlayerX;
    if (playerX < 10) playerX = 10; // 10 pixels reserved for score 
    else if (playerX >= tft.height() - 10) playerX = tft.height() - 10;

    playerY += map(sensorValueY, 30, 1000, maxSpeed, -maxSpeed);
    if (sensorValueY > 450 && sensorValueY < 550) playerY = previousPlayerY;
    if (playerY < 0) playerY = 0;
    else if (playerY >= tft.width() - 6) playerY = tft.width() - 6;
    
    updateScoreTimer();

    
    if (scoreTimer % 2 == 0 && !gos){
      gos = spawnStone(stoneStartSpeed, 5);    //  Stone logic needs work

      // Needs better spawning condition
      // Spawns at random width, at top.
      // After having spawned, it falls towards the bottom in a straight line
      // When it reaches bottom, it despawns
      // Rate increases over time.
      

    }


    if (gos){
      gos->moveStep();

      if (gos->y >= gos->maxFallLength){
        tft.fillRect(gos->x, gos->previousY, 10, 5, backgroundColor);
        hideTextLine(tft.height()-10);
        gos->backToTop(random(tft.width() - gos->size ));
      } 
      showStones();

      //If crash with player
      if (CrashedWithPlayer(gos)) gameOver();


    }






    // output

    showScoreTimer();




    hideText(previousPlayerY, previousPlayerX, 6);
    showText(playerY, playerX, "o", ST7735_WHITE);

    


    
  } else if (state == 2){
    // Game Two

  } else if (state == 3){
    // Game Three

  }
  delay(50);
}


void pressedJoystickButton(){
  if (state == 0){  // in main menu
    state = menuSelection +1; // +1 to offset from mainMenu state
  } else{
    state = 0;  //reset to menu state;
  }
}

void releasedJoystickButton(){
  hideTextLine(0);
}


// --- Main Menu Stuff --- //

void mainMenu(){
  updateMenuSelection();
  hideMainMenuSelections();
  showMainMenuSelection(); 
  delay(100);
}

void updateMenuSelection(){
  if (sensorValueX >= 900) menuSelection++;
  else if (sensorValueX <= 100) menuSelection--;
  if (menuSelection < 0) menuSelection = 0;
  if (menuSelection >= numberOfMenuSelections) menuSelection = numberOfMenuSelections-1;
}

// --- Game One Stuff --- //

void startGameOne(){
  showGameOneStartupAnimation();
  playerX = tft.height()/2 - 5;
  playerY = tft.width()/2 - 3;
  scoreTimer = 0;
  if (gos) {
    gos->resetStone(random(tft.width() - gos->size ), stoneStartSpeed);
  }
}

gameOneStone* spawnStone(int speed, int size){
  gameOneStone* gs = new gameOneStone(random(tft.width() - size), speed, size, tft.height());
  return gs;
}

bool CrashedWithPlayer(gameOneStone* _gos){
  if (_gos->x + 9 < playerY) return false;
  if (_gos->x > playerY + 5) return false;
  if (_gos->y + 9 < playerX) return false;
  if (_gos->y > playerX + 5) return false;

  return true;
}

void updateScoreTimer(){
  if (previousTime.second() != now.second()){
    hideText(8*5, 0, 6*5);
    previousTime = now;
    scoreTimer++;
  }
}

void showStones(){
  tft.fillRect(gos->x, gos->previousY, 10, 10, backgroundColor);
  tft.fillRect(gos->x, gos->y, 10, 10, ST7735_RED);
}

void gameOver(){
  showText(20, 90, "Game Over", ST7735_RED);
  //Stop movement
  //Stop stone(s)
  //Show score

}


// --- Show stuff --- //

void showMainMenu(){
  showText(15, 20, "Choose your Game", ST7735_BLUE);
  showText(20, 50, "First Game", ST7735_CYAN);
  showText(20, 70, "Second Game", ST7735_CYAN);
  showText(20, 90, "High Scores", ST7735_CYAN);
}

void showMainMenuSelection(){
  int y = 50 + (20*menuSelection);

  showText(5, y, ">", ST7735_WHITE);
}

void showNotImplemented(){
  showText(15, 40, "Not Implemented", ST7735_RED);
  showText(0, 80, "To Menu: Press button", ST7735_BLUE);
}

void showScoreTimer(){
  //hideTextLine(0);  //Optimize to hide only relevant square
  String s = "Score: ";
  s += scoreTimer;
  showTextLine(0, s, ST7735_YELLOW);
}


void showTextLine(int yValue, String text, uint16_t color) {
  showText(0, yValue, text, color);
}

void showText(int xValue, int yValue, String text, uint16_t color) {
  tft.setCursor(xValue, yValue);
  tft.setTextColor(color);
  tft.println(text);
}

// --- Hide stuff --- //

void hideMainMenuSelections(){
  for (int i = 0; i <= numberOfMenuSelections; i++){
    int y = 50 + (20 * i);
    hideText(5, y, 6);
  }
}

void hideText(int xStart, int yStart, int width) {
  tft.fillRect(xStart, yStart, width, 10 /*default font size*/, backgroundColor);
}

void hideTextLine(int yValue) {
  hideText(0, yValue, tft.width());
  //tft.fillRect(0, yValue, tft.width(), 10 /*default font size*/, backgroundColor);
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

void showGameOneStartupAnimation(){
  for (int16_t x=tft.width()-1; x > 6; x-=6) {
    tft.fillRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, ST7735_BLACK);
    tft.drawRect(tft.width()/2 -x/2, tft.height()/2 -x/2 , x, x, ST7735_BLUE);
  }
  tft.fillScreen(ST7735_BLACK);
}
