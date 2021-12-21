#include "LedControl.h" //  need the library
#include <binary.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
//LCD pins
const int RS = 7;
const int enable = A5;
const int d5 = 4;
const int d4 = 5;
const int d6 = 13;
const int d7 = 2;
const int contrastPin = 9;
int contrastLevel;// contrast = level * multiplier
int contrastMult = 10;
int contrastOffset = 50;
int contrastAddress = 60; //EEPROM address
const int brightnessPin = 10;
int LCDBrightnessLevel;
int brightnessMult = 20;
int LCDBrightnessOffset = 50;
int LCDBrightnessAddress = 70; //EEPROM address
LiquidCrystal lcd(RS,enable, d4, d5, d6, d7);
int lcdStateChange = 1;

//Matrix pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 8;
const int rows = 8;
const int cols = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER
const int matrixBrightnessOffset = 0;
int matrixBrightnessLevel = 6;
int matrixBrightnessAddress = 80;

//buzzer
int buzzPin = A3;
int buzzClickDuration = 50;
int menuClickSound = 6000;
int buzzChangeOptionDuration = 30;
int changeOptionSound = 8000;
int buzzChangeSettingDuration = 30;
int changeSettingSound = 4000;

//start menu screen related values
String texts[] = {"Welcome to", "Mike's BrickShot", "Click to enter"};
const int textSizes[] = {10, 16, 14};
const int nrTexts = 3;
int curText = 0;
String oneText;
const int charDisplayDelay = 150;
const int textDisplayDelay = 500;
//menu related values
int curOption = 0; //pozitia optiunii din menuOptions 
const int nrOptions = 5;
String menuOptions[] = {"Start Game", "High Scores", "Settings", "Change Name", "Delete scores"}; // optiunile de selectie din meniu
const int startGameOption = 0;
const int deleteScoresOption = 4;
String current(){
  return menuOptions[curOption];
}
String next(){
  //returneaza urmatoara pozitie din menuOptions
  int nextOpt = (curOption + 1) % nrOptions;
  // daca curOption e ultima pozitie, returneza prima pozitie
  return menuOptions[nextOpt];
}
//highscores screen
const int highScoresOption = 1;
int highScoreInFocus = 0;

//Setings screen
int soundSwitchPin = 6;
int soundSwitchValue;
int lastSoundSwitchValue = 0;
int pinSoundLED = 3;
int curSetting = 0;
int nrOfSettings = 4;
bool soundOn = 1;
int soundAddress = 90;//EEPROM address
String settingsMenu[] = {"Sound", "Contrast", "LCD bri.", "Matrix bri."};
const int settingsOption = 2;
const int changeNameOption = 3;
int nextSetting;

//game over screen
long gameOverTime = 0;
int gameOverScreenDelay = 3000;

//joystick pins
const int pinSW = A2;
const int pinX = A0;
const int pinY = A1;

//joystick values
bool switchState = HIGH;
bool lastSwitchState = HIGH;
int yValue = 0;
int xValue = 0;
int minThreshold = 350;
int maxThreshold = 700; 
bool joyMovedX = false;
bool joyMovedY = false;
bool joyUp = false;
bool joyDown = false;
bool joyLeft = false;
bool joyRight = false;
bool clicked = false;
long lastButtonPress = 0;
const long buttonPressDelay = 200;

//game stage values
int stage = 0;
const int welcomeScreenStage = 0;
const int enterNameStage1 = 1;
const int enterNameStage2 = 2;
const int gameMenuStage = 3;
const int highScoreStage = 4;
const int optionStage = 5;
const int gameplayStage = 6;
const int gameOverStage = 7;
const int settingStage = 8;

//game related values
int playerPos = 4;
String playerName = "";
int curNamePos = 0;
char curNameChar = ' ';
const int maxNameLen = 5;
int oldPlayerPos;
long score = 0;
int playerRow = 7;
int shotActive = 0;
int shotRow;
int shotCol;
int lastShotRow = 0;
const int shotDelay = 20;
long lastShotMove = 0;
long shotLandingRow = 0;
long gameplayStartTime = 0;
long gameplayTimeLimit = 10000;

//levels
int level = 1;
//                    0      1     2     3      4      5      6      7      8      9     10       11
int levelUpScore[]  = {0,   50,  100,  300,   700,  1200,  1600,  2000,  2500,  3000,  4500,  100000};
int levelUpAction[] = {0,    1,    2,    1,     2,     3,     1,     2,     1,     2,     1,       0};
int levelUpValues[] = {0, 3500,    5, 3100,     4,     1,  2700,     3,  2300,     2,  2000,       0};

//Actions:
//  1.increase speed
//  2.add fewer blocks on each new row
//  3.blocks start droping and hitting the player
//Values represent the new values given to the variables that represent the speed 
//and the number of blocks per row
int scoreMult = 10;
bool levelBlinking = false;
int levelBlinkDelay = 500;
long levelBlinkStartTime = 0;
long levelBlinkStopTime = 2000;
bool fallingBlocks = false;

//matrix and blocks
int maxBlocksRow = 0;// how many rows are actually occupied by blocks
int lastColumnNr = 7;// from 0 to 7
int lastRowNr = 6;// row nr 7 is the player row
int rowGeneratingTime = 4000;// time between generating two rows of blocks
long lastGeneratedRowTime = 0;
int blocksPerRow = 6;// no of blocks per generated row
int randomColumns[] = {0, 1, 2, 3, 4, 5, 6, 7}; // will be randomized for picking x random positions
//oneColumnAtATime[x] returns a byte array with a "1" on index x
long newRow;
const long oneColumnAtATime[] = {
         10000000,
          1000000,
           100000,
            10000,
             1000,
              100,
               10,
                1,
    };
long blankRow = 100000000;
long fullRow =  111111111;
long blockRows[] = {
        100000000,
        100000000,
        100000000,
        100000000,
        100000000,
        100000000,
        100000000,
    };

//eeprom and high scores
const int nrOfHighScores = 3;
int highScoresOffset = 4; // scorurile se afla pe pozitiile 0, 4 si 8
int highScoresNamesOffset = 20; // numele incep de pe pozitiile 20, 30 si 40
String highScoresNames[nrOfHighScores];
long highScores[nrOfHighScores];
int highScoresInitialized = 0;

//defining special characters
const uint8_t upArrow[] = {
        0b00100,
        0b01010,
        0b10101,
        0b00100,
        0b00100,
        0b00100,
        0b00000,
        0b00000,
    };
const uint8_t downArrow[] = {
        0b00000,
        0b00000,
        0b00100,
        0b00100,
        0b00100,
        0b10101,
        0b01010,
        0b00100,
    };
const uint8_t rightArrow[] = {
        0b00000,
        0b00000,
        0b00000,
        0b00100,
        0b00010,
        0b11101,
        0b00010,
        0b00100,
    };
const uint8_t leftArrow[] = {
        0b00000,
        0b00000,
        0b00000,
        0b00100,
        0b01000,
        0b10111,
        0b01000,
        0b00100,
    };
const uint8_t heartHollow[] = {
        0b01010,
        0b10101,
        0b10001,
        0b01010,
        0b00100,
        0b00000,
        0b00000,
        0b00000,
    };
const uint8_t heartFilled[] = {
        0b01010,
        0b11111,
        0b11111,
        0b01110,
        0b00100,
        0b00000,
        0b00000,
        0b00000,
    };

const uint8_t checked[] = {
        0b00000,
        0b00000,
        0b00001,
        0b00011,
        0b10110,
        0b11100,
        0b01000,
        0b00000,
    };
 
void setup()
{
  lcd.createChar (1, upArrow);
  lcd.createChar (2, downArrow);
  lcd.createChar (3, rightArrow);
  lcd.createChar (4, leftArrow);
  lcd.createChar (5, heartHollow);
  lcd.createChar (6, heartFilled);
  lcd.createChar (7, checked);
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  pinMode(soundSwitchPin, INPUT_PULLUP);
  pinMode(pinSoundLED, OUTPUT);
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(pinX, INPUT);
  pinMode(pinY, INPUT);
  pinMode(contrastPin, OUTPUT);
  Serial.begin(9600);
}


void movePlayer(){
  if(joyLeft == true){
      lc.setLed(0, playerRow, playerPos, false); // turns on LED at col, row
      if(playerPos > 0)
        playerPos -= 1;
      lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
    }
    if(joyRight == true){
      lc.setLed(0, playerRow, playerPos, false); // turns on LED at col, row
      if(playerPos < 7)
        playerPos += 1;
      lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
    }
}

void changeOption(){
  if(joyDown == true){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    curOption += 1;
    if(curOption == nrOptions)
      curOption = 0;
    lcdStateChange = 1;
  }
  if(joyUp == true){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    if(curOption > 0)
      curOption -= 1;
    else
      curOption = nrOptions - 1;
    lcdStateChange = 1;
  }
}

void printStartMenu(){
  lcd.clear();
  lcd.print ((char) 0x03);
  lcd.print(current());
  lcd.setCursor(15, 0);
  lcd.print ((char) 0x01);
  lcd.setCursor(15, 1);
  lcd.print ((char) 0x02);
  lcd.setCursor(1, 1);
  lcd.print(next());
}

// randomizes randomColumns array
void randomizeArray(){
  const int questionCount = sizeof randomColumns / sizeof randomColumns[0];
  for (int i=0; i < questionCount; i++) {
     int n = random(0, questionCount);  // Integer from 0 to questionCount-1
     int temp = randomColumns[n];
     randomColumns[n] =  randomColumns[i];
     randomColumns[i] = temp;
  }
}

void generateRow(){
  randomizeArray();
  newRow = blankRow;
  for(int i = 0; i < blocksPerRow; i++){ // take first blocksPerRow elements in array
    int x = randomColumns[i]; // random column
    newRow += oneColumnAtATime[x]; //adding 1 on that column
  }
  maxBlocksRow ++;//increase the number of occupied rows
  for(int j = maxBlocksRow; j > 0; j--){
    blockRows[j] = blockRows[j-1];
  }
  blockRows[0] = newRow;
}
//turns on/off just the blocks in game
void switchMatrix(bool val){
  //val == true  => turn on  matrix
  //val == false => turn off matrix
  long row;
  int ledVal;
  for(int i = 0; i <= maxBlocksRow; i++){
    row = blockRows[i];
    for(int j = lastColumnNr; j >= 0; j--){
      ledVal = row % 10;
      row = row / 10;
      if(ledVal){
        lc.setLed(0, i, j, val);
      }
    }
  }
  for(int k = maxBlocksRow + 1; k < lastColumnNr; k++){
    for(int l = 0; l <= lastColumnNr; l++){
      lc.setLed(0, k, l, false);
    }
  }
}
//turns on/off all leds of the matrix
void turnMatrix(bool val){
  //val == true  => turn on  matrix
  //val == false => turn off matrix
  for(int i = 0; i <= lastColumnNr; i++){
    for(int j = 0; j <= lastColumnNr; j++){
      lc.setLed(0, i, j, val);
    }
  }
  if(stage == gameplayStage){
    lc.setLed(0, playerRow, playerPos, true);
  }
}


void startGame(){
  score = 0;
  lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
  stage = gameplayStage;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.setCursor(7, 0);
  lcd.print(level);
  lcd.setCursor(13, 0);
  for(int i = 0; i < 3; i++){
    lcd.print ((char) 0x06);
  }
  lcd.setCursor(0, 1);
  lcd.print("Score:");
  lcd.setCursor(10, 1);
  lcd.print(score);
}

void play(){
  movePlayer();
  if(joyDown == true || maxBlocksRow > lastRowNr){
    gameOver();
  }
  if(score > levelUpScore[level]){
    if(levelUpAction[level] == 1){
      rowGeneratingTime = levelUpValues[level];
    }
    if(levelUpAction[level] == 2){
      blocksPerRow = levelUpValues[level];
    }
    if(levelUpAction[level] == 3){
      fallingBlocks = levelUpValues[level];
    }
    level++;
    levelBlinking = true;
    levelBlinkStartTime = millis();
  }
  if(levelBlinking){
    if(millis() % levelBlinkDelay > levelBlinkDelay / 2){
      lcd.setCursor(7, 0);
      lcd.print(level);
    }else{
      lcd.setCursor(7, 0);
      lcd.print("   ");
    }
    if(millis() - levelBlinkStartTime > levelBlinkStopTime){
      levelBlinking = false;
      lcd.setCursor(7, 0);
      lcd.print(level);
    }
    
  }
  if(shotActive){
    if(millis() - lastShotMove > shotDelay){
      lastShotMove = millis();
      lc.setLed(0, shotRow, shotCol, false);
      shotRow -= 1;
      if(shotRow >= shotLandingRow)
        lc.setLed(0, shotRow, shotCol, true);
      else{
        turnMatrix(false);
        addBlock();
        shotActive = 0;
        clearLine();
        switchMatrix(true);
      }
    }
  }
  if(clicked && !shotActive){
    clicked = false;
    shotRow = 6; // the row the shot appears on;
    shotCol = playerPos;
    shotLandingRow = calculateShotLandingRow();
    shotActive = 1;
    score++;
    lcd.setCursor(10, 1);
    lcd.print(score);
    lastShotMove = millis();
    lc.setLed(0, shotRow, shotCol, true);
  }
  if(millis() - lastGeneratedRowTime > rowGeneratingTime && !shotActive){
    switchMatrix(false);
    generateRow();
    lastGeneratedRowTime = millis();
    switchMatrix(true);
  }
  
}

void gameOver(){
  turnMatrix(false);
  lc.setLed(0, playerRow, playerPos, false);
  stage = gameOverStage;
  gameOverTime = millis();
  lcdStateChange = 1;
  uploadScore();
  for(int i = 0; i < lastColumnNr; i++){
    blockRows[i] = blankRow;
  }
  level = 1;
  levelBlinking = false;
  fallingBlocks = false;
  blocksPerRow = 6;
  rowGeneratingTime = 4000;
  playerPos = 4;
  shotActive = 0;
  maxBlocksRow = 0;
}

int calculateShotLandingRow(){
  int curRowNr = maxBlocksRow + 1;
  int block;
  long divider;
  while(curRowNr > 0){
    long nextRow = blockRows[curRowNr - 1];
    divider = power(10,(lastColumnNr - shotCol));
    block = nextRow/divider%10;
    if(block >= 1){
      return curRowNr;
    }else{
      curRowNr --;
    }
  }
  return curRowNr;
}

//calculate x to the power of y
long power(int x, int y){
  long result = 1;
  for (int i = 0; i < y; i++){
    result = result * x;
  }
  return result;
}

void addBlock(){
  if(shotLandingRow > maxBlocksRow){
    maxBlocksRow ++;
  }
  blockRows[shotLandingRow] += oneColumnAtATime[shotCol];
}

void clearLine(){
  for(int i = 0; i < lastColumnNr; i++){
    if(blockRows[i] == fullRow){
      score += level * scoreMult;
      lcd.setCursor(10, 1);
      lcd.print(score);
      switchMatrix(false);
      blockRows[i] = blankRow;
      for(int j = i; j < lastColumnNr - 1; j++){
        blockRows[j] = blockRows[j + 1];
      }
      blockRows[lastColumnNr - 1] = blankRow;
      switchMatrix(true);
      maxBlocksRow--;
    }
  }
}

void downloadHighScores(){
  for(int i = 0; i < nrOfHighScores; i++){
    EEPROM.get(i * highScoresOffset, highScores[i]);
    int hsNameLen = EEPROM.read((i + 1) * highScoresNamesOffset);
    char hsChar;
    String hsName = "";
    for(int j = 0; j < hsNameLen; j++){
      hsChar = EEPROM.read((i + 1) * highScoresNamesOffset + 1 + j);
      hsName = hsName + hsChar;
    }
    highScoresNames[i] = hsName;
  }
}

void downloadSettings(){
  contrastLevel = EEPROM.read(contrastAddress);
  LCDBrightnessLevel = EEPROM.read(LCDBrightnessAddress);
  matrixBrightnessLevel = EEPROM.read(matrixBrightnessAddress);
  soundOn = EEPROM.read(soundAddress);
  digitalWrite(pinSoundLED, soundOn);
  analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
  analogWrite(brightnessPin, LCDBrightnessOffset + brightnessMult * LCDBrightnessLevel);
  lc.setIntensity(0, matrixBrightnessLevel + matrixBrightnessOffset);
}

void uploadSettings(){
  EEPROM.put(contrastAddress, contrastLevel);
  EEPROM.write(LCDBrightnessAddress, LCDBrightnessLevel);
  EEPROM.write(matrixBrightnessAddress, matrixBrightnessLevel);
  EEPROM.write(soundAddress, soundOn);
}

void uploadScore(){
  if(score > highScores[2]){
    if(score > highScores[1]){
      if(score > highScores[0]){
        highScores[2] = highScores[1];
        highScoresNames[2] = highScoresNames[1];
        highScores[1] = highScores[0];
        highScoresNames[1] = highScoresNames[0];
        highScores[0] = score;
        highScoresNames[0] = playerName;
      }else{
        highScores[2] = highScores[1];
        highScoresNames[2] = highScoresNames[1];
        highScores[1] = score;
        highScoresNames[1] = playerName;
      }
    }else{
      highScores[2] = score;
      highScoresNames[2] = playerName;
    }
  }
  for(int i = 0; i < nrOfHighScores; i++){
    EEPROM.put(i * highScoresOffset, highScores[i]);
    //String hsName = highScoresNames[i];
    int hsNameLen = highScoresNames[i].length();
    char c;
    EEPROM.write((i + 1) * highScoresNamesOffset, hsNameLen);
    for(int j = 0; j < hsNameLen; j++){
      c = highScoresNames[i][j];
      EEPROM.write((i + 1) * highScoresNamesOffset + 1 + j, c);
    }
  }
}
//change the current character in the name selection screen
//the character is innitialized with " " (space)
void changeNameChar(){
  int c = int(curNameChar); //ASCII code of the character
  
  if(joyDown == true){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
      
      if(c == 32 || c == 65){ //ASCII code for space
        c = 90; //ASCII code for Z
      }
      else
        c --;
    }
    if(joyUp == true){
      if(soundOn){
        tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
      }
      
      if(c == 32 || c == 90){ //ASCII code for space
        c = 65; //ASCII code for A
      }
      else
        c ++;
    }
    curNameChar = char(c); //ASCII to char
    lcd.setCursor(curNamePos, 1);
    lcd.print(curNameChar);
}



void showHighScores(){
  if(clicked){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    stage = gameMenuStage;
    printStartMenu();
    clicked = false;
  }
  if(lcdStateChange){
    lcdStateChange = 0;
    lcd.clear();
    //first row
    lcd.setCursor(0, 0);
    lcd.print(highScoreInFocus + 1);
    lcd.print(".");
    lcd.print(highScoresNames[highScoreInFocus]);
    lcd.setCursor(8, 0);
    lcd.print(":");
    lcd.print(highScores[highScoreInFocus]);
    //second row
    lcd.setCursor(0, 1);
    lcd.print(highScoreInFocus + 2);
    lcd.print(".");
    lcd.print(highScoresNames[highScoreInFocus + 1]);
    lcd.setCursor(8, 1);
    lcd.print(":");
    lcd.print(highScores[highScoreInFocus + 1]);
    }
  if(joyUp && highScoreInFocus > 0){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    
    highScoreInFocus --;
    lcdStateChange = 1;
  }
  if(joyDown && highScoreInFocus < nrOfHighScores - 2){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    highScoreInFocus ++;
    lcdStateChange = 1;
  }
}

void joyStickListener(){
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  switchState = digitalRead(pinSW);
  soundSwitchValue = digitalRead(soundSwitchPin);
  //x-axys
  if(xValue < minThreshold && joyMovedX == false){
      joyDown = true;
      joyMovedX = true;
    }
    if(xValue > maxThreshold && joyMovedX == false){
      joyUp = true;
      joyMovedX = true;
    }
    if (xValue > minThreshold && xValue < maxThreshold){
      joyMovedX = false;
      joyUp = false;
      joyDown = false;
    }
    
  //y-axys
  if(yValue < minThreshold && joyMovedY == false){
      joyLeft = true;
      joyMovedY = true;
    }
    if(yValue > maxThreshold && joyMovedY == false){
      joyRight = true;
      joyMovedY = true;
    }
    if (yValue > minThreshold && yValue < maxThreshold){
      joyMovedY = false;
      joyLeft = false;
      joyRight = false;
    }
  // joystick switch
  if(switchState == HIGH && lastSwitchState == LOW){
    clicked = true;
  }
  lastSwitchState = switchState;

  // sound switch
  if(soundSwitchValue == LOW && lastSoundSwitchValue == HIGH){
    soundOn = !soundOn;
    EEPROM.write(soundAddress, soundOn);
    digitalWrite(pinSoundLED, soundOn);
    if(stage == settingStage){
      if(curSetting == 0){
        lcd.setCursor(13, 0);
        lcd.print("   ");
        lcd.setCursor(13, 0);
        if(soundOn){
          lcd.print("ON");
        }else
          lcd.print("OFF");
      }
      if(curSetting == 3){
        lcd.setCursor(13, 1);
        lcd.print("   ");
        lcd.setCursor(13, 1);
        if(soundOn){
          lcd.print("ON");
        }else
          lcd.print("OFF");
      }
    }
  }
  lastSoundSwitchValue = soundSwitchValue;
  lastSwitchState = switchState;
}

void gameOverScreen(){
  if(lcdStateChange){
    lcdStateChange = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("GAME OVER");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
  }
  if(clicked && millis() - gameOverTime > gameOverScreenDelay){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    
    stage = gameMenuStage;
    lcdStateChange = 1;
    score = 0;
    
    clicked = false;
  }
  if(millis() - gameOverTime > gameOverScreenDelay){
    if(clicked){
      stage = gameMenuStage;
      lcdStateChange = 1;
      score = 0;
    }
    lcd.setCursor(15, 0);
    lcd.print ((char) 0x07);
  }
}

void setingsScreen(){
  if(clicked){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    turnMatrix(false);
    uploadSettings();
    stage = gameMenuStage;
    clicked = false;
    printStartMenu();
  }
  if(lcdStateChange){
    lcdStateChange = 0;
    lcd.clear();
    //first row
    lcd.setCursor(0, 0);
    lcd.print((char) 0x03);
    lcd.setCursor(1, 0);
    lcd.print(settingsMenu[curSetting]);
    lcd.setCursor(13, 0);
    if(curSetting == 0){ //If the setting is Sound
        if(soundOn)
            lcd.print("ON");
        else
            lcd.print("OFF");
    }
    else
        lcd.print(settingValue(curSetting));
    // second row
    nextSetting = (curSetting + 1) % nrOfSettings;
    lcd.setCursor(1, 1);
    lcd.print(settingsMenu[nextSetting]);
    lcd.setCursor(13, 1);
    if(nextSetting == 0){ //If the setting is Sound
        if(soundOn)
            lcd.print("ON");
        else
            lcd.print("OFF");
    }
    else
        lcd.print(settingValue(nextSetting));
  }
  turnMatrix(curSetting == 3);
  if(joyUp){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    
    if(curSetting == 0)
      curSetting = nrOfSettings - 1;
    else
      curSetting--;
    lcdStateChange = 1;
  }
  if(joyDown){
    if(soundOn){
      tone(buzzPin, changeOptionSound, buzzChangeOptionDuration);
    }
    
    curSetting = nextSetting;
    lcdStateChange = 1;
  }
  if(joyRight){
    if(curSetting == 0){
      soundOn = !soundOn;
      digitalWrite(pinSoundLED, soundOn);
    }
    if(curSetting == 1 && contrastLevel < 10){
      contrastLevel ++;
      analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
    }
    if(curSetting == 2 && LCDBrightnessLevel < 10){
      LCDBrightnessLevel ++;
      analogWrite(brightnessPin, LCDBrightnessOffset + brightnessMult * LCDBrightnessLevel);
    }
    if(curSetting == 3 && matrixBrightnessLevel < 10){
      matrixBrightnessLevel ++;
      lc.setIntensity(0, matrixBrightnessLevel + matrixBrightnessOffset);
    }
    lcdStateChange = 1;
    if(soundOn){
      tone(buzzPin, changeSettingSound, buzzChangeSettingDuration);
    }
  }
  if(joyLeft){
    if(curSetting == 0){
      soundOn = !soundOn;
      digitalWrite(pinSoundLED, soundOn);
    }
    if(curSetting == 1 && contrastLevel > 0){
      contrastLevel --;
      analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
    }
    if(curSetting == 2 && LCDBrightnessLevel > 0){
      LCDBrightnessLevel --;
      analogWrite(brightnessPin, LCDBrightnessOffset + brightnessMult * LCDBrightnessLevel);
    }
    if(curSetting == 3 && matrixBrightnessLevel > 0){
      matrixBrightnessLevel --;
      lc.setIntensity(0, matrixBrightnessLevel + matrixBrightnessOffset);
    }
    lcdStateChange = 1;
    if(soundOn){
      tone(buzzPin, changeSettingSound, buzzChangeSettingDuration);
    }
  }
}

int settingValue(int settingIndex){
    if(settingIndex == 1)
        return contrastLevel;
    if(settingIndex == 2)
        return LCDBrightnessLevel;
    if(settingIndex == 3)
        return matrixBrightnessLevel;
}

void deleteHighScores(){
  for(int i = 0; i < nrOfHighScores; i++){
    highScoresNames[i] = "";
    highScores[i] = 0;
  }
  uploadScore();
  lcd.setCursor(0, 1);
  lcd.print ((char) 0x07);
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print (" ");
}

void initialDownload(){
  if(!highScoresInitialized){
    downloadHighScores();
    downloadSettings();
    highScoresInitialized = 1;
  }
}

void welcomeScreen(){
  while(curText < nrTexts){
      if(curText > 1){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(oneText);
      }
      String oneText = texts[curText];
      for(int i = 0; i < textSizes[curText]; i++){
        lcd.print(oneText[i]);
        delay(charDisplayDelay);
      }
      curText += 1;
      lcd.setCursor(0, 1);
    }
    if(clicked){
      if(soundOn){
        tone(buzzPin, menuClickSound, buzzClickDuration);
      }
      
      lcd.clear();
      stage = enterNameStage1;
    }
}

void initializeEnterNameScreen(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter name:");
  lcd.setCursor(curNamePos + 1, 1);
  lcd.print((char) 0x04);
  lcd.setCursor(curNamePos, 1);
  lcd.print(curNameChar);
  stage = enterNameStage2;
}

void enterName(){
  changeNameChar();
  if(clicked){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    if(curNameChar == ' '){
      stage = gameMenuStage;
    }else{
      playerName = playerName + curNameChar;
      curNameChar = ' ';
      curNamePos++;
      lcd.setCursor(curNamePos, 1);
      lcd.print(curNameChar);
      lcd.print((char) 0x04);
    }
    clicked = false;
  }
  if(playerName.length() == maxNameLen)
    stage = gameMenuStage;
}

void gameMenu(){
  if(lcdStateChange == 1){
    printStartMenu();
    lcdStateChange = 0;
  }
  if(millis()%10000 > 9000){
  }
  
  if(clicked){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    if(curOption == startGameOption){
      startGame();
    }
    if(curOption == highScoresOption){
      highScoreInFocus = 0;
      stage = highScoreStage;
      lcdStateChange = 1;
    }
    if(curOption == changeNameOption){
      lcdStateChange = 1;
      playerName = "";
      curNameChar = ' ';
      curNamePos = 0;
      stage = enterNameStage1;
    }
    if(curOption == settingsOption){
      lcdStateChange = 1;
      stage = settingStage;
    }
    if(curOption == deleteScoresOption){
      deleteHighScores();
    }
  }
  changeOption();
}

void joyStickReset(){
  joyUp = false;
  joyDown = false;
  joyLeft = false;
  joyRight = false;
  clicked = false;
}

void loop(){
  joyStickListener();
  initialDownload();
  switch(stage){
    case welcomeScreenStage:
      welcomeScreen();
      break;
    case enterNameStage1:
      initializeEnterNameScreen();
      break;
    case enterNameStage2:
      enterName();
      break;
    case gameplayStage:
      play();
      break;
    case highScoreStage:
      showHighScores();
      break;
    case gameOverStage:
      gameOverScreen();
      break;
    case settingStage:
      setingsScreen();
      break;
    case gameMenuStage:
      gameMenu();
      break;
  }
  joyStickReset();
}
