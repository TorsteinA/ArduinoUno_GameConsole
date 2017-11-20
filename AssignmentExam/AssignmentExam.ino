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

int state = 0;  // 0 = main Menu, 1 = game1, 2 = game2, 3 = high Scores, 4 = gameOverMenu (some go to not-implemented screen);
int previousState;
const int STATE_MAIN_MENU = 0;
const int STATE_GAME_ONE = 1;
const int STATE_GAME_TWO = 2;
const int STATE_HIGH_SCORES = 3;
const int STATE_GAME_OVER = 4;


// Main Menu
int menuSelection = 0;
int numberOfMenuSelections = 3;

// Game One
const int maxSpeed = 10;
int stoneStartSpeed = 2;
int stoneStartSize = 4;
gameOneStone* gos = NULL;

// Games
int playerX, previousPlayerX, playerY, previousPlayerY;
int scoreTimer = 0;
bool paused = false;
int pauseMenuSelection = 0;
int numberOfPauseMenuSelections = 2;

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
    } else if (state == STATE_GAME_OVER) {
      gameOverMenu();
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
    if (!paused){
      playGameOne();
    } else {
      pauseMenu();
    }
    
  } else if (state == STATE_GAME_TWO) {
    // Game Two

  } else if (state == STATE_HIGH_SCORES) {
    // High Scores

  } else if (state == STATE_GAME_OVER) {
    // Game Over Menu

  }
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

// --- Game One Stuff --- //

void startGameOne() {
  showGameOneStartupAnimation();
  playerX = tft.height() / 2 - 5;
  playerY = tft.width() / 2 - 3;
  scoreTimer = 0;
  paused = false;
  if (gos) {
    gos->resetStone(random(tft.width() - gos->size ), stoneStartSpeed, stoneStartSize);
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
  if (!gos) {
    gos = spawnStone(stoneStartSpeed, stoneStartSize);
  }
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

void gameOverMenu() {
  hideTextLine(0);
  showText(30, 40, "Game Over", ST7735_RED);
  String s = "Score: ";
  s += scoreTimer;
  showText(30, 80, s, ST7735_GREEN);
  showText(0, 120, "To Menu: Press button", ST7735_BLUE);

}

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
  for (int i = 0; i <= numberOfPauseMenuSelections; i++) {
    int y = 70 + (20 * i);
    hideText(5, y, 6);
  }
}

void showPauseMenuSelection(){
  int y = 70 + (20 * pauseMenuSelection);

  showText(5, y, ">", ST7735_WHITE);
}



// --- Show stuff --- //

void showPauseMenu() {
  showText(15, 20, "Paused", ST7735_RED);
  showText(20, 70, "Resume", ST7735_CYAN);
  showText(20, 90, "Back to Menu", ST7735_CYAN);
}

void showMainMenu() {
  showText(15, 20, "Choose your Game", ST7735_BLUE);
  showText(20, 50, "Don't get hit", ST7735_CYAN);
  showText(20, 70, "Through the slit", ST7735_CYAN);
  showText(20, 90, "High Scores", ST7735_WHITE);
}

void showMainMenuSelection() {
  int y = 50 + (20 * menuSelection);

  showText(5, y, ">", ST7735_WHITE);
}

void showNotImplemented() {
  showText(15, 40, "Not Implemented", ST7735_RED);
  showText(0, 80, "To Menu: Press button", ST7735_BLUE);
}

void showScoreTimer() {
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

void hidePauseMenu(){
  hideText(15, 40, 6 * 6);
}

void hideMainMenuSelections() {
  for (int i = 0; i <= numberOfMenuSelections; i++) {
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
  tft.fillScreen(backgroundColor);
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color1);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, color2);
  }
  //Show name of system (The Adventures of O) on top
  //Delay
  delay(500);
}

void showGameOneStartupAnimation() {
  for (int16_t x = tft.width() - 1; x > 6; x -= 6) {
    tft.fillRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, ST7735_BLACK);
    tft.drawRect(tft.width() / 2 - x / 2, tft.height() / 2 - x / 2 , x, x, ST7735_BLUE);
  }
  // Show title of game on top
  // Delay
  delay(400);
  tft.fillScreen(backgroundColor);
}
