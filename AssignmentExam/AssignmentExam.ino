#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include <Wire.h> // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
//#include "pitches.h"
#include "gameOneStone.h"

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
const uint8_t TFT_CS   = 10;
const uint8_t TFT_RST  = 8;
const uint8_t TFT_DC   = 9;
const uint8_t TFT_SCLK = 13;
const uint8_t TFT_MOSI = 11;
const uint8_t analogInPinY = A0;
const uint8_t analogInPinX = A1;
const uint8_t buttonPin = 2;
const uint8_t chipSelect = 4;

uint16_t sensorValueY = 0;
uint16_t sensorValueX = 0;
uint8_t buttonState = 0;          // current state of the button
uint8_t lastButtonState = 0;      // previous state of the button


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
const uint16_t backgroundColor = ST7735_BLACK;

RTC_DS1307 rtc;
DateTime now, previousTime;

File myFile;


uint8_t state = 0;  // 0 = main Menu, 1 = game1, 2 = game2, 3 = high Scores, 4 = showGameOverMenu (some go to not-implemented screen);
uint8_t previousState;
const uint8_t STATE_MAIN_MENU = 0;
const uint8_t STATE_GAME_ONE = 1;
const uint8_t STATE_GAME_TWO = 2;
const uint8_t STATE_HIGH_SCORES = 3;
const uint8_t STATE_GAME_OVER = 4;


// Main Menu
uint8_t menuSelection = 0;
uint8_t numberOfMenuSelections = 3;


// Game One
const uint8_t maxSpeed = 10;
uint8_t stoneStartSpeed = 2;
uint8_t stoneStartSize = 4;
gameOneStone* gos = NULL;
int highScoresOne[] = {5,6,7,8};


// Games
uint16_t playerX, previousPlayerX, playerY, previousPlayerY;
uint16_t scoreTimer = 0;
bool paused = false;
uint8_t pauseMenuSelection = 0;
uint8_t numberOfPauseMenuSelections = 2;
const uint8_t tempLoadArraySize = 8;

void setup(void) {

  pinMode(buttonPin, INPUT);
  pinMode(analogInPinY, INPUT);
  pinMode(analogInPinX, INPUT);
  randomSeed(analogRead(0));

  while (!Serial); // for Leonardo/Micro/Zero

  //Serial.begin(57600);
  Serial.begin(9600);

  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization successfull.");
  


  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(2); // Flips the output upside down. This is due to how I placed the hardware

  //runSDSetup();

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

  runStartUpAnimation(ST7735_BLACK, ST7735_GREEN);
  tft.fillScreen(ST7735_BLACK);

  showMainMenu();

  Serial.println("Initialized");
  
}

void loop() {
  now = rtc.now();
  buttonState = digitalRead(buttonPin);
  sensorValueY = analogRead(analogInPinY);
  sensorValueX = analogRead(analogInPinX);

  // Check button input
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      pressedJoystickButton();
    } else {
      releasedJoystickButton();
    }
    delay(100);
    lastButtonState = buttonState;
  }

  // Check State Update
  if (state != previousState) {
    if (state == STATE_MAIN_MENU) {
      tft.fillScreen(backgroundColor);
      showMainMenu();
    } else if (state == STATE_GAME_ONE) {
      tft.fillScreen(backgroundColor);
      startGameOne();
    } else if (state == STATE_GAME_TWO) {
      tft.fillScreen(backgroundColor);
      startGameTwo();
    } else if (state == STATE_HIGH_SCORES) {
      tft.fillScreen(backgroundColor);
      showHighScoreMenu();
    } else if (state == STATE_GAME_OVER) {
      showGameOverMenu();
    } else {
      tft.fillScreen(backgroundColor);
      showNotImplemented();
    }
    previousState = state;
  }

  // Display
  if (state == STATE_MAIN_MENU) {
    mainMenu();
    
  } else if (state == STATE_GAME_ONE) {
    if (paused){
      pauseMenu();
    } else {
      playGameOne();
    }
    
  } else if (state == STATE_GAME_TWO) {
    // Game Two

  } else if (state == STATE_HIGH_SCORES) {
    // High Scores

  }/* else if (state == STATE_GAME_OVER) {
    
  }*/
  delay(50);
}


void pressedJoystickButton() {
  if (state == STATE_MAIN_MENU) { // in main menu
    state = menuSelection + 1; // +1 to offset from mainMenu state
  } else if (state == STATE_GAME_ONE) { // in game one
    if (paused && pauseMenuSelection == 0) {
      // Resume Game by:
        // Hide pause stuff or clear screen
        // Paused = false;

      tft.fillScreen(backgroundColor);
      paused = false;
      
    } else if (paused && pauseMenuSelection == 1){
      // Exit to Menu and save score by:
        // hide pause stuff or clear screen
        // paused = false
        // save score
        // state = game over

      tft.fillScreen(backgroundColor);
      paused = false;
      // Save Score
      state = STATE_MAIN_MENU;
      
    } else if (!paused) {
      paused = true;
      showPauseMenu();
    }

  } else {
    state = STATE_MAIN_MENU;  //reset to menu state;
  }
}

void releasedJoystickButton() {
  //hideTextLine(0);
}


// --- Main Menu Stuff --- //

void mainMenu() {
  updateMenuSelection();
  hideMainMenuSelections();
  showMainMenuSelection();
  delay(100);
}

void updateMenuSelection() {
  if (sensorValueX >= 900) menuSelection++;
  else if (sensorValueX <= 100) menuSelection--;
  if (menuSelection < 0) menuSelection = 0;
  if (menuSelection >= numberOfMenuSelections) menuSelection = numberOfMenuSelections - 1;
}

void showMainMenu() {
  tft.setTextSize(2);
  showText(10, 20, "Main Menu", ST7735_GREEN);
  tft.setTextSize(1);

  showText(20, 65, "Don't get hit", ST7735_CYAN);
  showText(20, 85, "Through the slit", ST7735_CYAN);
  showText(20, 105, "High Scores", ST7735_CYAN);
}

void showMainMenuSelection() {
  uint8_t y = 65 + (20 * menuSelection);

  showText(5, y, ">", ST7735_WHITE);
}

void hideMainMenuSelections() {
  for (uint8_t i = 0; i <= numberOfMenuSelections; i++) {
    uint8_t y = 65 + (20 * i);
    hideText(5, y, 6);
  }
}

// --- Pause Menu stuff --- //


void pauseMenu(){
  updatePauseMenuSelection();
  hidePauseMenuSelections();
  showPauseMenuSelection();
  delay(100);
}

void updatePauseMenuSelection(){
  if (sensorValueX >= 900) pauseMenuSelection++;
  else if (sensorValueX <= 100) pauseMenuSelection--;
  if (pauseMenuSelection < 0) pauseMenuSelection = 0;
  if (pauseMenuSelection >= numberOfPauseMenuSelections) pauseMenuSelection = numberOfPauseMenuSelections - 1;
}

void hidePauseMenuSelections(){
  for (uint8_t i = 0; i <= numberOfPauseMenuSelections; i++) {
    uint8_t y = 70 + (20 * i);
    hideText(5, y, 6);
  }
}

void showPauseMenuSelection(){
  uint8_t y = 70 + (20 * pauseMenuSelection);

  showText(5, y, ">", ST7735_WHITE);
}

void showPauseMenu() {
  // Square of backgroundColor behind Paused Text
  tft.drawRoundRect(9, 17, 111, 27, 5, ST7735_RED);
  tft.setTextSize(3);
  showText(12, 20, "Paused", ST7735_RED);
  tft.setTextSize(1);
  showText(20, 70, "Resume", ST7735_CYAN);
  showText(20, 90, "Back to Menu", ST7735_CYAN);
}

void hidePauseMenu(){
  hideText(15, 40, 6 * 6);
}


// --- Game Over Menu stuff --- //

void showGameOverMenu() {
  hideTextLine(0);
  // Square of backgroundColor behind GameOver Text  
  tft.drawRoundRect(26, 11, 76, 54, 5, ST7735_RED);
  tft.setTextSize(3);
  showText(30, 15, "Game", ST7735_RED);
  showText(30, 40, "Over", ST7735_RED);
  tft.setTextSize(1);
  String s = "Score: ";
  s += scoreTimer;

  showText(35, 75, s, ST7735_GREEN);
  
  showText(5, 110, ">", ST7735_WHITE);
  showText(25, 110, "Back to Menu", ST7735_CYAN);

}


// --- Game One Stuff --- //

void startGameOne() {
  showGameOneStartupAnimation();
  playerX = tft.height() / 2 - 5;
  playerY = tft.width() / 2 - 3;
  scoreTimer = 0;
  paused = false;
  if (gos) {
    gos->resetStone(random(tft.width() - gos->size ), stoneStartSpeed, stoneStartSize);
  } else {
    gos = spawnStone(stoneStartSpeed, stoneStartSize);
  }
}

void playGameOne() {
  // Input
  updatePlayerInput();

  // Logic
  updateScoreTimer();
  updateGameOneStone();
  
  if (CrashedWithPlayer(gos)) {
    state = STATE_GAME_OVER;
    //Save Score to file
    SaveGameOneScore(scoreTimer);
  }

  // Output
  showUpdateStones();
  showScoreTimer();
  showUpdatePlayerPos();
}

gameOneStone* spawnStone(int speed, int size) {
  return new gameOneStone(random(tft.width() - size), speed, size, tft.height());
}

bool CrashedWithPlayer(gameOneStone* _gos) {
  if (_gos->x + /*9*/ _gos->size - 1 < playerY) return false;
  if (_gos->x > playerY + 5) return false;
  if (_gos->y + /*9*/ _gos->size - 1 < playerX) return false;
  if (_gos->y > playerX + 5) return false;

  return true;
}

void updateGameOneStone(){
  gos->moveStep();

  if (gos->y >= gos->maxFallLength) {
    tft.fillRect(gos->x, gos->previousY, gos->size, gos->size, backgroundColor);
    hideTextLine(tft.height() - 10);
    gos->backToTop(random(tft.width() - gos->size ));
  }

}

void updateScoreTimer() {
  if (previousTime.second() != now.second()) {
    hideText(8 * 5, 0, 6 * 5);
    previousTime = now;
    scoreTimer++;
  }
}

void updatePlayerInput() {
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

}

void showUpdatePlayerPos(){
  hideText(previousPlayerY, previousPlayerX, 6);
  showText(playerY, playerX, "o", ST7735_WHITE);
}

void showUpdateStones() {
  tft.fillRect(gos->x, gos->previousY, gos->size, gos->size, backgroundColor);
  tft.fillRect(gos->x, gos->y, gos->size, gos->size, ST7735_RED);
}

void showScoreTimer() {
  String s = "Score: ";
  s += scoreTimer;
  showTextLine(0, s, ST7735_YELLOW);
}


// --- Game Two stuff --- //

void startGameTwo(){
  showGameTwoStartupAnimation();
  playerX = tft.height() / 2 - 5;
  playerY = 30;
  scoreTimer = 0;
  paused = false;

  // if wall, resetWall
  // else, spawnWall





  //Remove later:
  tft.fillScreen(backgroundColor);
  showNotImplemented();
  

}


// --- Show other stuff --- //

void showNotImplemented() {
  showText(15, 40, "Not Implemented", ST7735_RED);
  showText(0, 80, "To Menu: Press button", ST7735_BLUE);
}

void showTextLine(int yValue, String text, uint16_t color) {
  showText(0, yValue, text, color);
}

void showText(int xValue, int yValue, String text, uint16_t color) {
  tft.setCursor(xValue, yValue);
  tft.setTextColor(color);
  tft.println(text);
}

void showHighScoreMenu(){

  updateHighScores();

  showText(10, 10, "Scores:", ST7735_CYAN);


  for (int i = 0; i <= sizeof(highScoresOne)/sizeof(highScoresOne[0])-1; i++){
    String s = "";
    s += highScoresOne[i];
    showText(20, 20 + (10*i), s, ST7735_BLUE);
  }
  
  showText(10, 110, "Done reading.", ST7735_CYAN);

}

// --- Hide stuff --- //

void hideText(int xStart, int yStart, int width) {
  tft.fillRect(xStart, yStart, width, 10 /*default font size*/, backgroundColor);
}

void hideTextLine(int yValue) {
  hideText(0, yValue, tft.width());
  //tft.fillRect(0, yValue, tft.width(), 10 /*default font size*/, backgroundColor);
}


// --- Animations --- //

void runStartUpAnimation(uint16_t color1, uint16_t color2) {
  tft.fillScreen(backgroundColor);

  uint8_t color = 100;
  uint8_t i;
  uint8_t t;
  for(t = 0 ; t <= 4; t+=1) {
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t w = tft.width()-2;
    uint8_t h = tft.height()-2;
    for(i = 0 ; i <= 16; i+=1) {
      tft.drawRoundRect(x, y, w, h, 5, color);
      x+=2;
      y+=3;
      w-=4;
      h-=6;
      color+=1100;
    }
    color+=100;
  }
  delay(100);
  tft.setTextSize(4);
  showText(30, 17, "The", ST7735_GREEN);
  tft.setTextSize(2);
  showText(5, 58, "Adventures", ST7735_GREEN);
  tft.setTextSize(3);
  showText(48, 80, "of", ST7735_GREEN);
  tft.setTextSize(5);
  showText(50, 110, "O", ST7735_GREEN);
  tft.setTextSize(1);
  delay(2000);
}

void showGameOneStartupAnimation() {
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, ST7735_BLACK);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, ST7735_BLUE);
  }
  delay(100);
  tft.setTextSize(4);
  showText(7, 23, "Don't", ST7735_RED);
  showText(30, 63, "Get", ST7735_RED);
  showText(30, 103, "Hit", ST7735_RED);
  tft.setTextSize(1);
  delay(1500);
  tft.fillScreen(backgroundColor);
}

void showGameTwoStartupAnimation(){
  uint8_t color = 0xF800;
  uint8_t t;
  uint8_t w = tft.width()/2;
  uint8_t x = tft.height()-1;
  uint8_t y = 0;
  uint8_t z = tft.width();
  for(t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
  delay(100);
  tft.setTextSize(2);
  showText(23, 35, "Through", ST7735_GREEN);
  tft.setTextSize(3);
  showText(38, 67, "The", ST7735_GREEN);
  showText(30, 105, "Slit", ST7735_GREEN);
  tft.setTextSize(1);
  delay(1500);
  tft.fillScreen(backgroundColor);
}

// --- SD Card methods --- //

void updateHighScores(){
  //Load all values from SD card ("game1.txt") into tempArray
  //Sort tempArray in decreasing order
  //Fill highScoresOne array with elements from tempArray
  //Clear file ("game1.txt")
  //Save top scores in order


  
  
  int* score;//[tempLoadArraySize];

  Serial.print("Score created.");
  Serial.println(score[0]);
  
  LoadGameOneScore(&score);

  Serial.print("Score returned. Now: ");
  Serial.println(score[0]);
  
  
  // Sort temp Array

  
  for(byte i = 0; i <= sizeof(highScoresOne)/sizeof(highScoresOne[0]); i++){
    highScoresOne[i] = score[i];
  }
}

void SaveGameOneScore(uint16_t score) { 
  myFile = SD.open("game1.txt", FILE_WRITE);
  
  if (myFile) {
    Serial.print("Writing to game1.txt...");
    
    myFile.println(score);
    myFile.close();
    
    Serial.println("closed game1.txt");
  } else {
    Serial.println("error opening game1.txt");
  }
}

void LoadGameOneScore(int **score){
//  int score[tempLoadArraySize];
  int i = 0;
  Serial.println("Reading from game1.txt");

  myFile = SD.open("game1.txt");
  
  if (myFile) {
    Serial.println("opened game1.txt");

    // read from the file until there's nothing else in it:
    //while (myFile.available() && i <= sizeof(highScoresOne)/sizeof(highScoresOne[0] -1) ) {
    //while (i <= 4) {
    //while (i <= sizeof(highScoresOne)/sizeof(highScoresOne[0]) && myFile.available())
    while (i <= tempLoadArraySize && myFile.available())
    {
      Serial.print("Reading value from game1..");
      int k = myFile.read();
      Serial.print(k);
      //Serial.write(myFile.read());
      *score[i] = k;//myFile.read();   // Set back to k
      //highScoresOne[i] = k;
      i++;
    }

    myFile.close();
    Serial.println("closed game1.txt");
  } else {
    Serial.println("error opening game1.txt");
  }
  //return score;
}