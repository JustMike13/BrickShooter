/*
    loop function:
The functionality is divided in stages. Each stage dictates a different part of the game/menu.
I call a function inside the loop function to register any joystick movement or button press. For each direction (up, down, left, right) and the button I save the coresponding value (true or false) in a variable.
At the end of the loop function, I set every one of those variables to false, so the value does not translate into the next call.
    Scrollable menus:
I save the current option (the one in focus) is a variable and I print that one on the first line of the LCD and the next one on the second line.
If I register a joystick movement (joyUp or joyDown are true) I increase or decrease the option by 1 and print again.
I use lcdStateChange variable to decide when to print.
    Main menu:
When a click is registered I switch to the stage coresponding to the current option.
    Settings:
If joyLeft or joyRight are true I change the value of the setting in focus by 1.
The sound setting can also be changed from the hardware button.
When a click is registered I switch back to the main menu.
    EEPROM:
For each value that is saved in EEPROM, I assigned a fixed address.
For the names, I save the length of the name on the assigned address and each character on the next ones.
I read the values at the start of the program and write them each time a change has been made.
    Gameplay:
The player is fixed to the bottom row of the matrix. joyLeft and joyRight move the player.
Rows of random positioned blocks are beeing generated and set at the top of the screen. Each row generated pushes the other ones down one row.
When a click is registered the shotActive variable is set to true, the landing position of the shot is calculated and the shot's position is set right over the player.
While shotActive is true the shot is moved, one row at a time, on the same column, until it reaches it's landing position and shotActive is set to false.
After a certain level blocks from the lowest row start falling. The mechanic is similar to shooting, just moving towards the player.
If the player gets hit by the blocks he loses a life.
When the row is full it disappears and all rows that were lower are moved up.




*/

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
int contrastLevel;// contrast = level * multiplier + offset
int contrastMult = 10;
int contrastOffset = 50;
int contrastAddress = 80; //EEPROM address
const int brightnessPin = 10;
int LCDBrightnessLevel;
int brightnessMult = 20;
int LCDBrightnessOffset = 50;
int LCDBrightnessAddress = 85; //EEPROM address
LiquidCrystal lcd(RS,enable, d4, d5, d6, d7);
int lcdStateChange = 1;
int LCDColumnNo = 16;
int LCDRowNo = 2;

//Matrix pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 8;
const int rows = 8;
const int cols = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER
const int matrixBrightnessOffset = 0;// Brightness = level * multiplier + offset
int matrixBrightnessLevel = 6;
int matrixBrightnessAddress = 90;

//buzzer
int buzzPin = A3;
int buzzClickDuration = 50;
int menuClickSound = 6000;
int buzzChangeOptionDuration = 30;
int changeOptionSound = 8000;
int buzzChangeSettingDuration = 30;
int changeSettingSound = 4000;
int buzzMovePlayerDuration = 20;
int movePlayerSound = 3000;
long shotTime = 0;
int shotSoundOffset = 4000;
int shotSoundMult = 20;
int shotSoundDuration = 100;
bool shotSoundActive = false;
int buzzClearLineSoundDuration = 100;
int clearLineSound = 500;

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
const int nrOptions = 7;
String menuOptions[] = {"Start Game", "High Scores", "Settings", "Change Name", "Delete scores", "About", "Instructions"}; // optiunile de selectie din meniu
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
int soundAddress = 95;//EEPROM address
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
unsigned int lastDebounceTime = 0; 
unsigned int debounceDelay = 50; 
int reading = HIGH;
int previousReading = HIGH;

//game stage values
int stage = 0;
const int welcomeScreenStage = 0;
const int enterNameStage1 = 1;
const int enterNameStage2 = 2;
const int gameMenuStage = 3;
const int highScoreStage = 4;
const int gameplayStage = 5;
const int gameOverStage = 6;
const int settingStage = 7;
const int aboutStage = 8;
const int instructionsStage = 9;

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
int playerLives = 3;
int livePosition = 13;

//falling blocks
int fallingCol;
int fallingRow;
bool fallingActive = 0;
int fallingChanceCalculatingDelay = 2000;
long lastChanceCalculatedTime = 0;
int fallingChance = 3; //30%
int curChance = 9;
int fallingBlockDelay = 300;
long lastBlockFall = 0;
long fallingBlockRow;
int dropBlockIndex = 0;
long dropBlockDivider = 1;
int dropBlockPower = 0;
int dropBlockValue = 0;

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
bool fallingBlocksEnabled = false;

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

//about screen
int aboutOption = 5;
int aboutMaxRow = 4;
int curAboutPos = 0;
String aboutRows[] = {"Author: ", "   Mihai Ristea", "Contact: ", "   mihai.ristea", "@s.unibuc.ro"};

//instructions screen
int instructionsOption = 6;
int instructionsMaxRow = 8;
int curInstructionsPos = 0;
String instructionsRows[] = {"The player can",
                            "shoot blocks in",
                            "the empty",
                            "spaces. When a",
                            "line is full it",
                            "disappears. The",
                            "game ends when",
                            "the lines reach",
                            "the player."};

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
  lcd.begin(LCDColumnNo, LCDRowNo);
  lcd.createChar (1, upArrow);
  lcd.createChar (2, downArrow);
  lcd.createChar (3, rightArrow);
  lcd.createChar (4, leftArrow);
  lcd.createChar (5, heartHollow);
  lcd.createChar (6, heartFilled);
  lcd.createChar (7, checked);
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

//Changes the position of the player according to the moves registered by the joystick
void movePlayer(){
  if(joyLeft == true){
    lc.setLed(0, playerRow, playerPos, false); // turns on LED at col, row
    if(playerPos > 0)
      playerPos -= 1;
    lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
    if(soundOn){
      tone(buzzPin, movePlayerSound, buzzMovePlayerDuration);
    }
  }
  if(joyRight == true){
    lc.setLed(0, playerRow, playerPos, false); // turns on LED at col, row
    if(playerPos < 7)
      playerPos += 1;
    lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
    if(soundOn){
      tone(buzzPin, movePlayerSound, buzzMovePlayerDuration);
    }
  }
}

//Changes the current option according to the moves registered by the joystick
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

//Prints the main menu to the lcd
void printStartMenu(){
  lcd.clear();
  lcd.print ((char) 0x03);
  lcd.print(current());//the option in focus
  lcd.setCursor(15, 0);
  lcd.print ((char) 0x01);
  lcd.setCursor(15, 1);
  lcd.print ((char) 0x02);
  lcd.setCursor(1, 1);
  lcd.print(next());//the next option
}

// shuffles randomColumns array
void randomizeArray(){
  const int questionCount = sizeof randomColumns / sizeof randomColumns[0];
  for (int i=0; i < questionCount; i++) {
     int n = random(0, questionCount);  // Integer from 0 to questionCount-1
     int temp = randomColumns[n];
     randomColumns[n] =  randomColumns[i];
     randomColumns[i] = temp;
  }
}
//takes the first blocksPerRow elements of the shuffled array
//and adds blocks on the columns determined by these elements to a blank row
//a blank row is a number formed by a 1 and 8 zeros (100.000.000)
//oneColumnAtATime[x] returnes a number that, when added to a row, adds 1 to the x column of that row
//example: if my row is 100100111 and I want to add a 1 on the 4th column (index 3 on the matrix) I add oneColumnAtATime[3], which return 10000
//and i get:    100100111 +
//                   1000
//      =       100101111
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

//turns on/off all leds of the matrix, except the player in the gameplay stage
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

//prints heart icons in the corner of the lcd
void printLives(){
  lcd.setCursor(livePosition, 0);
  lcd.print("   ");
  lcd.setCursor(livePosition, 0);
  for(int i = 0; i < playerLives; i++){
    lcd.print ((char) 0x06);
  }
}

//prints the score, level and name of the player to the lcd and sets the gameplay stage value
void startGame(){
  score = 0;
  lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
  stage = gameplayStage;
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if(playerName == ""){
    lcd.print("name");
  }else{
    lcd.print(playerName);
  }
  lcd.setCursor(6, 0);
  lcd.print("lvl.");
  lcd.setCursor(10, 0);
  lcd.print(level);
  printLives();
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  
  lcd.setCursor(8, 1);
  lcd.print(score);
}

//everything related to the actual game, including falling blocks, moving the player, shooting, leveling up and stoping the game when a losing condition is met
void play(){
  //moving the player
  movePlayer();
  //if the player touches the falling block he/she loses a life
  if(fallingCol == playerPos && fallingRow == playerRow && fallingActive){
    lc.setLed(0, playerRow, playerPos, true); // because the falling blocks will disappear and it will turn off the led that shows the player
    fallingActive = 0;
    playerLives--;
    printLives();
  }
  //moves the falling block down or calculates the chance for a block to drop every few seconds
  if(fallingActive){
    fallingBlock();
  }else if(fallingBlocksEnabled && millis() - lastChanceCalculatedTime > fallingChanceCalculatingDelay){
    lastChanceCalculatedTime = millis();
    curChance = random(0, 10);
  }
  // if the calculated chance is lower than the actual chance it drops one block
  if(curChance < fallingChance){
    dropBlock();
  }
  //game over conditions
  if(joyDown == true || maxBlocksRow > lastRowNr || playerLives == 0){
    gameOver();
  }
  //leveling up
  if(score > levelUpScore[level]){
    if(levelUpAction[level] == 1){
      rowGeneratingTime = levelUpValues[level];
    }
    if(levelUpAction[level] == 2){
      blocksPerRow = levelUpValues[level];
    }
    if(levelUpAction[level] == 3){
      fallingBlocksEnabled = true;
    }
    level++;
    levelBlinking = true;
    levelBlinkStartTime = millis();
  }
  //after a level up the level number on the lcd blinks for a few seconds
  if(levelBlinking){
    if(millis() % levelBlinkDelay > levelBlinkDelay / 2){
      lcd.setCursor(10, 0);
      lcd.print(level);
    }else{
      lcd.setCursor(10, 0);
      lcd.print("   ");
    }
    if(millis() - levelBlinkStartTime > levelBlinkStopTime){
      levelBlinking = false;
      lcd.setCursor(10, 0);
      lcd.print(level);
    }
  }
  //if a shot has been fired and it didn't reach it's landing position it is moved one row up
  //when the shot reaches it's landing position the block is added to the row
  //if the row is filled it is destroyed
  if(shotActive){
    if(millis() - shotTime < shotSoundDuration){
      tone(buzzPin, shotSoundOffset - (millis() - shotTime) * shotSoundMult);
    }
    else if(shotSoundActive){
      shotSoundActive = false;
      noTone(buzzPin);
    }
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
  //if a click is registered I initialize a shot 
  if(clicked && !shotActive){
    clicked = false;
    if(soundOn){
      shotTime = millis();
      shotSoundActive = true;
    }
    shotRow = 6; // the row the shot appears on;
    shotCol = playerPos; // player's column
    shotLandingRow = calculateShotLandingRow();
    if(shotLandingRow == maxBlocksRow){
      maxBlocksRow++;//if the block lands on a clear row, I increment the number of occupied rows
    }
    shotActive = 1;
    score++;
    lcd.setCursor(8, 1);
    lcd.print(score);
    lastShotMove = millis();
    lc.setLed(0, shotRow, shotCol, true);
  }
  //generate a row every few seconds
  if(millis() - lastGeneratedRowTime > rowGeneratingTime && !shotActive){
    switchMatrix(false);
    generateRow();
    lastGeneratedRowTime = millis();
    switchMatrix(true);
  }
}

//
void dropBlock(){
  curChance = 9;//set chance to 9 so I don't enter the if statement again
  randomizeArray();//I shuffle the array
  fallingRow = maxBlocksRow - 1;
  fallingBlockRow = blockRows[fallingRow]; // the last row with blocks on it
  do{//find a random index of a block on that row
    fallingCol = random(0, 8);
    dropBlockPower = lastColumnNr - fallingCol;
    dropBlockDivider = power(10, dropBlockPower);
    dropBlockValue = fallingBlockRow/dropBlockDivider%10;
  } while(dropBlockValue == 0);
  blockRows[fallingRow] -= dropBlockDivider;//delete the block from the row
  fallingActive = 1;//innitiate falling
  //if the row only had one block and it falls, decrease the number of rows
  if(blockRows[fallingRow] == blankRow){
    maxBlocksRow --; 
  }
}

//move the block one row down, stop when it reaches the last row 
//turn led off, change position and turn back on
void fallingBlock(){
  if(millis() - lastBlockFall > fallingBlockDelay){
    lastBlockFall = millis();
    lc.setLed(0, fallingRow, fallingCol, false);
    if(fallingRow <= lastRowNr){
      fallingRow ++;
      lc.setLed(0, fallingRow, fallingCol, true);
    }
    else {
      // if(fallingCol == playerPos){
      //   playerLives--;
      //   printLives();
      // }
      fallingActive = 0;
    }
  }
}

//shuts down the matrix, resets values to defaults and uploads the scores
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
  fallingBlocksEnabled = false;
  blocksPerRow = 6;
  rowGeneratingTime = 4000;
  playerPos = 4;
  shotActive = 0;
  maxBlocksRow = 0;
  fallingActive = 0;
  playerLives = 3;
  curChance = 9;
}

//when a shot is fired I look through every row, when I find a block on the shot's column I set the landing positon on the last row
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

//adds a block on the landing position of the shot
void addBlock(){
  if(shotLandingRow > maxBlocksRow){
    maxBlocksRow ++;
  }
  blockRows[shotLandingRow] += oneColumnAtATime[shotCol];
}

//if a row is equal to the full row it is deleted, every row that was beneath it moves up
//adds 10 times the level to the score
void clearLine(){
  for(int i = 0; i < lastColumnNr; i++){
    if(blockRows[i] == fullRow){
      if(soundOn){
        tone(buzzPin, clearLineSound, buzzClearLineSoundDuration);
      }
      score += level * scoreMult;
      lcd.setCursor(8, 1);
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

// download from EEPROM
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
// download from EEPROM
void downloadSettings(){
  EEPROM.get(contrastAddress, contrastLevel);
  LCDBrightnessLevel = EEPROM.read(LCDBrightnessAddress);
  matrixBrightnessLevel = EEPROM.read(matrixBrightnessAddress);
  soundOn = EEPROM.read(soundAddress);
  digitalWrite(pinSoundLED, soundOn);
  analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
  analogWrite(brightnessPin, LCDBrightnessOffset + brightnessMult * LCDBrightnessLevel);
  lc.setIntensity(0, matrixBrightnessLevel + matrixBrightnessOffset);
}
//upload to EEPROM
void uploadSettings(){
  EEPROM.write(contrastAddress, contrastLevel);
  EEPROM.write(LCDBrightnessAddress, LCDBrightnessLevel);
  EEPROM.write(matrixBrightnessAddress, matrixBrightnessLevel);
  EEPROM.write(soundAddress, soundOn);
}

//upload to EEPROM
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


//prints the highScore in focus on the first row and the next one on the second
//when clicked it switches back to the main menu
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

//reads the values of the joystick and saves them in variables
void joyStickListener(){
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  reading = digitalRead(pinSW);
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
  if(reading != previousReading){
    lastDebounceTime = millis();
  }
  if (millis() - lastDebounceTime > debounceDelay){
    if (reading != switchState){
      switchState = reading;
      if (switchState == HIGH){
        clicked = true;
      }
    }
  }
  previousReading = reading;

  // sound switch
  // when pressed it changes the value of the sound setting
  // if in the setting stage it also prints the value on the lcd
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
}

// prints the game over screen, after a delay if clicked it returns to the main menu
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
  //after a few seconds show a 'v' in the corner of the lcd and allows the player to go back to the main menu with a click
  if(millis() - gameOverTime > gameOverScreenDelay){
    if(clicked){
      if(soundOn){
        tone(buzzPin, menuClickSound, buzzClickDuration);
      }
      stage = gameMenuStage;
      lcdStateChange = 1;
      score = 0;
    }
    lcd.setCursor(15, 0);
    lcd.print ((char) 0x07);
  }
}

//shows setting in focus and next one
void setingsScreen(){
  if(clicked){
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    turnMatrix(false);
    uploadSettings();//upload the settings to EEPROM before going back to main menu
    stage = gameMenuStage;
    clicked = false;
    printStartMenu();
  }
  //if a change has been registered print the updated screen
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
  // if the setting in focus is matrixBrightness turn on the matrix
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
//returns the value of the respective setting
int settingValue(int settingIndex){
    if(settingIndex == 1)
        return contrastLevel;
    if(settingIndex == 2)
        return LCDBrightnessLevel;
    if(settingIndex == 3)
        return matrixBrightnessLevel;
}
//deletes all highscores 
void deleteHighScores(){
  for(int i = 0; i < nrOfHighScores; i++){
    highScoresNames[i] = "name";
    highScores[i] = 0;
  }
  uploadScore();
  lcd.setCursor(0, 1);
  lcd.print ((char) 0x07);
  delay(500);//I do want to stop the system for a moment while i also show a small confirm icon
  lcd.setCursor(0, 1);
  lcd.print (" ");
}
//downloades settings and high scores from EEPROM when the system is turned on
void initialDownload(){
  if(!highScoresInitialized){
    downloadHighScores();
    downloadSettings();
    highScoresInitialized = 1;
  }
}
//prints the welcome animation
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
//prints the static elements of the enter name screen
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
  changeNameChar();//allows changing the current character
  //when a click is registered it saves the character and it moves to the next
  if(clicked){
    clicked = false;
    if(soundOn){
      tone(buzzPin, menuClickSound, buzzClickDuration);
    }
    //if the current character was a space when the player pressed the button it saves the current name
    if(curNameChar == ' '){
      stage = gameMenuStage;
    }else{//add the character to the name
      playerName = playerName + curNameChar;
      curNameChar = ' ';
      curNamePos++;
      lcd.setCursor(curNamePos, 1);
      lcd.print(curNameChar);
      lcd.print((char) 0x04);
    }
  }
  //if the name has reached maximum lenght save the name 
  if(playerName.length() == maxNameLen)
    stage = gameMenuStage;
}

//print the main menu
//if a click is registered innitiate the selected option
void gameMenu(){
  if(lcdStateChange == 1){
    printStartMenu();
    lcdStateChange = 0;
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
    if(curOption == aboutOption){
      lcdStateChange = 1;
      curAboutPos = 0;
      stage = aboutStage;
    }
    if(curOption == instructionsOption){
      lcdStateChange = 1;
      curInstructionsPos = 0;
      stage = instructionsStage;
    }
  }
  changeOption();
}
//print the about screen
//click goes back to main menu
void aboutScreen(){
  if(lcdStateChange){
    lcd.clear();
    lcd.setCursor(LCDColumnNo - 1, 0);
    lcd.print((char) 0x01);
    lcd.setCursor(LCDColumnNo - 1, 1);
    lcd.print((char) 0x02);
    lcd.setCursor(0, 0);
    lcd.print(aboutRows[curAboutPos]);
    lcd.setCursor(0, 1);
    lcd.print(aboutRows[curAboutPos + 1]);
    lcdStateChange = 0;
  }
  if(joyDown && curAboutPos < aboutMaxRow - 1){
    curAboutPos ++;
    lcdStateChange = 1;
  }
  if(joyUp && curAboutPos > 0){
    curAboutPos --;
    lcdStateChange = 1;
  }
  if(clicked){
    lcdStateChange = 1;
    stage = gameMenuStage;
  }
}

//print the instructions screen
//click goes back to main menu
void instructionsScreen(){
  if(lcdStateChange){
    lcd.clear();
    lcd.setCursor(LCDColumnNo - 1, 0);
    lcd.print((char) 0x01);
    lcd.setCursor(LCDColumnNo - 1, 1);
    lcd.print((char) 0x02);
    lcd.setCursor(0, 0);
    lcd.print(instructionsRows[curInstructionsPos]);
    lcd.setCursor(0, 1);
    lcd.print(instructionsRows[curInstructionsPos + 1]);
    lcdStateChange = 0;
  }
  if(joyDown && curInstructionsPos < instructionsMaxRow - 1){
    curInstructionsPos ++;
    lcdStateChange = 1;
  }
  if(joyUp && curInstructionsPos > 0){
    curInstructionsPos --;
    lcdStateChange = 1;
  }
  if(clicked){
    lcdStateChange = 1;
    stage = gameMenuStage;
  }
}
//resets the values so they don't translate to the next call of loop
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
    case aboutStage:
      aboutScreen();
      break;
    case instructionsStage:
      instructionsScreen();
      break;
  }
  joyStickReset();
}
