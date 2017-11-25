#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include <Wire.h> // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include "gameOneStone.h"
#include "gameTwoPipe.h"

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


uint8_t state = 0;
uint8_t previousState;
const uint8_t STATE_MAIN_MENU = 0;
const uint8_t STATE_GAME_ONE = 1;
const uint8_t STATE_GAME_TWO = 2;
const uint8_t STATE_HIGH_SCORES = 3;
const uint8_t STATE_GAME_OVER = 4;

const char titleThe[] PROGMEM = "The";
const char titleAdv[] PROGMEM = "Adventures";
const char titleOf[] PROGMEM = "of";
const char titleO[] PROGMEM = "O";
const char mainMenuTitle[] PROGMEM = "Main Menu";
const char mainMenuGameOne[] PROGMEM = "Don't get hit";
const char mainMenuGameTwo[] PROGMEM = "Through the slit";
const char pauseTitle[] PROGMEM = "Paused";
const char pauseResume[] PROGMEM = "Resume";
const char pauseBack[] PROGMEM = "Back to Menu";  //reused for gameOver screen
const char gameOverGame[] PROGMEM = "GAME";
const char gameOverOver[] PROGMEM = "OVER";
const char gameOverScore[] PROGMEM = "Score: ";   //reused in games
const char saveFileOne[] PROGMEM = "game1.txt";
const char saveFileTwo[] PROGMEM = "game2.txt";
const char saveSuccess[] PROGMEM = "Score saved";
const char animDont[] PROGMEM = "Don't";
const char animGet[] PROGMEM = "Get";
const char animHit[] PROGMEM = "Hit";
const char animThrough[] PROGMEM = "Through";
const char animThe[] PROGMEM = "The";
const char animSlit[] PROGMEM = "Slit";
const char errorSave[] PROGMEM = "Error saving score";  
const char errorSD[] PROGMEM = "SD init failed";
const char errorRestart[] PROGMEM = "Please restart";
const char errorRtc[] PROGMEM = "RTC Error";

const char* const string_table[] PROGMEM = {
  titleThe,        // 0
  titleAdv,        // 1
  titleOf,         // 2
  titleO,          // 3
  mainMenuTitle,   // 4
  mainMenuGameOne, // 5
  mainMenuGameTwo, // 6
  pauseTitle,      // 7
  pauseResume,     // 8
  pauseBack,       // 9
  gameOverGame,    // 10
  gameOverOver,    // 11
  gameOverScore,   // 12
  saveFileOne,     // 13
  saveFileTwo,     // 14
  saveSuccess,     // 15
  animDont,        // 16
  animGet,         // 17
  animHit,         // 18
  animThrough,     // 19
  animThe,         // 20
  animSlit,        // 21
  errorSave,       // 22
  errorSD,         // 23
  errorRestart,    // 24
  errorRtc,        // 25
};

char buffer[24];


// Main Menu
short menuSelection = 0;
uint8_t numberOfMenuSelections = 2;


// Game One
uint8_t stoneStartSpeed = 2;
uint8_t stoneStartSize = 4;
gameOneStone* gos = NULL;


// Game Two
const uint8_t pipeStartSpeed = 1;
const uint8_t pipeHoleStartSize = 40;
gameTwoPipe* pipe = NULL;


// Games
short playerX;
short playerY;
const uint8_t maxPlayerSpeed = 10;
uint8_t previousPlayerX;
uint8_t previousPlayerY; 
uint8_t scoreOffset = 10; 
uint16_t scoreTimer = 0;
bool paused = false;
short pauseMenuSelection = 0;
uint8_t numberOfPauseMenuSelections = 2;
const uint8_t tempLoadArraySize = 8;

void setup(void) {

  pinMode(buttonPin, INPUT);
  pinMode(analogInPinY, INPUT);
  pinMode(analogInPinX, INPUT);
  randomSeed(analogRead(0));

  //while (!Serial); // for Leonardo/Micro/Zero

  Serial.begin(9600);
  
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.setRotation(2); // Flips the output upside down. This is due to how I placed the hardware
  
  if (!SD.begin(chipSelect)) {
    tft.fillScreen(backgroundColor);
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[23]))); //sd init error
    showText(25, 40, buffer, ST7735_RED);
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[24]))); //please restart
    showText(25, 50, buffer, ST7735_RED);
    while(1);
    return;
  }
    

  if (! rtc.begin()) {
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[25]))); //rtc error
    showText(25, 40, buffer, ST7735_RED);
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[24]))); //please restart
    showText(25, 50, buffer, ST7735_RED);
    while (1);
  }
  if (! rtc.isrunning()) {
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[25]))); //rtc error
    showText(25, 40, buffer, ST7735_RED);
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[24]))); //please restart
    showText(25, 50, buffer, ST7735_RED);
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  previousTime = now;

  runStartUpAnimation(backgroundColor, ST7735_GREEN);
  tft.fillScreen(backgroundColor);

  showMainMenu();
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
    } else if (state == STATE_GAME_OVER) {
      showGameOverMenu();
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
    if (paused){
      pauseMenu();
    } else {
      playGameTwo();
    }
  }
  delay(50);
}


void pressedJoystickButton() {
  if (state == STATE_MAIN_MENU) { 
    state = menuSelection + 1; // +1 to offset from mainMenu state
  } else if (state == STATE_GAME_ONE) {
    if (paused){
      tft.fillScreen(backgroundColor);
      paused = false;
      if (pauseMenuSelection == 1){
        SaveGameOneScore();
        state = STATE_MAIN_MENU;
      }
    } else{
      paused = true;
      showPauseMenu();
    }
  } else if (state == STATE_GAME_TWO) {
    if (paused){
      tft.fillScreen(backgroundColor);
      paused = false;
      if (pauseMenuSelection == 1){
        SaveGameTwoScore();
        state = STATE_MAIN_MENU;
      }
    } else{
      paused = true;
      showPauseMenu();
    }
  } else {
    state = STATE_MAIN_MENU;
  }
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
  if (menuSelection >= numberOfMenuSelections) {
    menuSelection = numberOfMenuSelections - 1;
  }
}

void showMainMenu() {
  tft.setTextSize(2);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[4]))); //main menu
  showText(10, 20, buffer, ST7735_GREEN);
  tft.setTextSize(1);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[5]))); //don't get hit
  showText(20, 65, buffer, ST7735_CYAN);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[6]))); //through the slit
  showText(20, 85, buffer, ST7735_CYAN);
}

void showMainMenuSelection() {
  showText(5, 65 + (20 * menuSelection), ">", ST7735_WHITE);
}

void hideMainMenuSelections() {
  for (uint8_t i = 0; i <= numberOfMenuSelections; i++) {
    hideText(5, 65 + (20 * i), 6);
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
  for (uint8_t i = 0; i <= numberOfPauseMenuSelections-1; i++) {
    uint8_t y = 70 + (20 * i);
    hideText(5, y, 6);
  }
}

void showPauseMenuSelection(){
  showText(5, 70 + (20 * pauseMenuSelection), ">", ST7735_WHITE);
}

void showPauseMenu() {
  tft.drawRoundRect(9, 17, 111, 27, 5, ST7735_RED);
  tft.setTextSize(3);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[7]))); //paused
  showText(12, 20, buffer, ST7735_RED);
  tft.setTextSize(1);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[8]))); //resume
  showText(20, 70, buffer, ST7735_CYAN);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[9]))); //back to menu
  showText(20, 90, buffer, ST7735_CYAN);
}

void hidePauseMenu(){
  hideText(15, 40, 6 * 6);
}


// --- Game Over Menu stuff --- //

void showGameOverMenu() {
  hideTextLine(0);
  tft.drawRoundRect(26, 11, 76, 54, 5, ST7735_RED);
  tft.setTextSize(3);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[10]))); //game
  showText(30, 15, buffer, ST7735_RED);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[11]))); //over
  showText(30, 40, buffer, ST7735_RED);
  tft.setTextSize(1);
  
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[12]))); //score:
  sprintf(buffer, "%s%d", buffer, scoreTimer);
  showText(35, 75, buffer, ST7735_BLUE);
  
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[9]))); //back to menu
  showText(27, 110, buffer, ST7735_CYAN);

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
    gos = new gameOneStone(random(tft.width() - stoneStartSize), 
      stoneStartSpeed, stoneStartSize, tft.height());
  }
}

void playGameOne() {
  // Input
  updatePlayerInput();

  // Logic
  updateScoreTimer(true);
  updateGameOneStone();

  // Output
  showUpdateStones();
  showScoreTimer();
  showUpdatePlayerPos();
  
  // Lose condition
  if (crashedWithPlayer(gos)) {
    state = STATE_GAME_OVER;
    SaveGameOneScore();
  }
}

bool crashedWithPlayer(gameOneStone* _gos) {
  if (_gos->x + _gos->size - 1 < playerY) return false;
  if (_gos->x > playerY + 5) return false;
  if (_gos->y + _gos->size - 1 < playerX) return false;
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

void showUpdateStones() {
  tft.fillRect(gos->x, gos->previousY, gos->size, gos->size, backgroundColor);
  tft.fillRect(gos->x, gos->y, gos->size, gos->size, ST7735_RED);
}


// --- Game Two stuff --- //

void startGameTwo(){
  showGameTwoStartupAnimation();
  playerX = tft.height() / 2 - 5;
  playerY = 30;
  scoreTimer = 0;
  paused = false;

  if (pipe) {
    pipe->resetPipe(10 + random(tft.height() - 10 - pipeHoleStartSize), pipeHoleStartSize, pipeStartSpeed);
  } else {
    pipe = new gameTwoPipe(tft.height(), tft.width(), 
    pipeStartSpeed, pipeHoleStartSize, 10 + random(tft.height() - 10 - pipeHoleStartSize));
  }
}

void playGameTwo(){
  // Input
  updatePlayerInput();

  // Logic
  updateScoreTimer(false);
  updateGameTwoPipe();

  // Output
  showUpdatePipes();
  showScoreTimer();
  showUpdatePlayerPos();
  
  // Lose condition
  if (crashedWithPlayer(pipe)){
    state = STATE_GAME_OVER;
    SaveGameTwoScore();
  }
}

bool crashedWithPlayer(gameTwoPipe* _pipe){
  if (playerY > pipe->x + pipe->pipeWidth) return false;
  if (playerY + 5 < pipe->x) return false;
  if (playerX + 2 > pipe->holePosY + scoreOffset){
    if(playerX + 2 + 5 < pipe->holePosY + pipe->holeSize + scoreOffset) return false;
  }
  return true;
}

void updateGameTwoPipe(){
  pipe->moveStep();
  if (pipe->x <= 0){
    pipe->x = 0;
  }
  if(pipe->previousX <= 0){
    tft.fillRect(0, scoreOffset, pipe->pipeWidth*2, pipe->screenHeight - scoreOffset, backgroundColor);
    pipe->backToStart(random(tft.height() - pipe->holeSize - scoreOffset));
  }
}

void showUpdatePipes(){
  uint8_t diff = pipe->previousX - pipe->x;
  tft.fillRect(pipe->x + pipe->pipeWidth, scoreOffset, diff, pipe->screenHeight - scoreOffset, backgroundColor);
  tft.fillRect(pipe->x, scoreOffset, diff, pipe->holePosY, ST7735_GREEN);
  uint8_t y = pipe->holePosY + pipe->holeSize + scoreOffset;
  tft.fillRect(pipe->x, y, diff, pipe->screenHeight - y, ST7735_GREEN);
}


// --- Games --- //

void updateScoreTimer(bool gameOne) {
  if (previousTime.second() != now.second()) {
    hideText(8 * 5, 0, 6 * 5);
    previousTime = now;
    
    uint8_t scoreIncrease;
    if(gameOne){
      scoreIncrease = map(playerX, scoreOffset, tft.height(), 5, 0); 
    } else {
      scoreIncrease = map(playerY, 0, tft.width(), 1, 5); 
    }

    scoreTimer += scoreIncrease;
  }
}

void updatePlayerInput() {
  previousPlayerX = playerX;
  previousPlayerY = playerY;

  playerX += map(sensorValueX, 30, 1000, -maxPlayerSpeed, maxPlayerSpeed);
  if (sensorValueX > 470 && sensorValueX < 530) playerX = previousPlayerX;
  if (playerX < 10) playerX = 10; // 10 pixels reserved for score
  else if (playerX >= tft.height() - 10) playerX = tft.height() - 10;

  playerY += map(sensorValueY, 30, 1000, maxPlayerSpeed, -maxPlayerSpeed);
  if (sensorValueY > 450 && sensorValueY < 550) playerY = previousPlayerY;
  if (playerY <= 0) playerY = 0;
  else if (playerY >= tft.width() - 6) playerY = tft.width() - 6;
}

void showScoreTimer() {
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[12]))); //score:
  sprintf(buffer, "%s%d", buffer, scoreTimer);
  showTextLine(0, buffer, ST7735_YELLOW);
}

void showUpdatePlayerPos(){
  tft.fillRect(previousPlayerY, previousPlayerX+2, 5, 5, backgroundColor);
  showText(playerY, playerX, "o", ST7735_WHITE);
}


// --- Show Text --- //

void showTextLine(int yValue, char text[], uint16_t color) {
  showText(0, yValue, text, color);
}

void showText(int xValue, int yValue, char text[], uint16_t color) {
  tft.setCursor(xValue, yValue);
  tft.setTextColor(color);
  tft.println(text);
}


// --- Hide Text --- //

void hideText(int xStart, int yStart, int width) {
  tft.fillRect(xStart, yStart, width, 10 /*default font size*/, backgroundColor);
}

void hideTextLine(int yValue) {
  hideText(0, yValue, tft.width());
}


// --- Animations --- //

void runStartUpAnimation(uint16_t color1, uint16_t color2) {
  tft.fillScreen(backgroundColor);

  uint8_t color = 100;
  uint8_t i, t;
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
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[0])));  //the
  showText(30, 17, buffer, ST7735_GREEN);
  tft.setTextSize(2);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[1])));  //adventures
  showText(5, 58, buffer, ST7735_GREEN);
  tft.setTextSize(3);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[2])));  //of
  showText(48, 80, buffer, ST7735_GREEN);
  tft.setTextSize(5);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[3])));  //o
  showText(50, 110, buffer, ST7735_GREEN);
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
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[16])));  //dont
  showText(7, 23, buffer, ST7735_RED);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[17])));  //get
  showText(30, 63, buffer, ST7735_RED);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[18])));  //hit
  showText(30, 103, buffer, ST7735_RED);
  tft.setTextSize(1);
  delay(1500);
  tft.fillScreen(backgroundColor);
}

void showGameTwoStartupAnimation(){
  uint8_t color = 0xF800;
  uint8_t w = tft.width()/2;
  uint8_t x = tft.height()-1;
  uint8_t y = 0;
  uint8_t z = tft.width();
  for(uint8_t t = 0 ; t <= 15; t++) {
    tft.drawTriangle(w, y, y, x, z, x, color);
    x-=4;
    y+=4;
    z-=4;
    color+=100;
  }
  delay(100);
  tft.setTextSize(2);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[19])));  //through
  showText(23, 35, buffer, ST7735_GREEN);
  tft.setTextSize(3);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[20])));  //the
  showText(38, 67, buffer, ST7735_GREEN);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[21])));  //slit
  showText(30, 105, buffer, ST7735_GREEN);
  tft.setTextSize(1);
  delay(1500);
  tft.fillScreen(backgroundColor);
}


// --- SD Card methods --- //

void SaveGameOneScore(){
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[13])));  //fileName 1
  myFile = SD.open(buffer, FILE_WRITE);
  
  if (myFile) {
    myFile.println(scoreTimer);
    myFile.close();
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[15]))); //score saved
    showText(30, 140, buffer, ST7735_GREEN);
  } else {
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[22]))); //error saving
    showText(10, 140, buffer, ST7735_RED);
  }
  delay(300);
}

void SaveGameTwoScore(){
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[14])));  //fileName 2
  myFile = SD.open(buffer, FILE_WRITE);
  
  if (myFile) {
    myFile.println(scoreTimer);
    myFile.close();
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[15]))); //score saved
    showText(30, 140, buffer, ST7735_GREEN);
  } else {
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[22]))); //error saving
    showText(10, 140, buffer, ST7735_RED);
  }
  delay(300);
}