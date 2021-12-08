#include "LedControl.h" //  need the library
#include <LiquidCrystal.h>
#include <EEPROM.h>
//LCD pins
const int RS = 7;
const int enable = 6;
const int d5 = 4;
const int d4 = 5;
const int d6 = 3;
const int d7 = 2;
const int contrastPin = 9;
int contrastLevel;// contrast = level * multiplier
int contrastMult = 10;
int contrastOffset = 50;
int contrastAddress = 50; //EEPROM address
const int brightnessPin = 10;
int brightnessLevel;
int brightnessMult = 20;
int brightnessOffset = 50;
int brightnessAddress = 60; //EEPROM address
LiquidCrystal lcd(RS,enable, d4, d5, d6, d7);
int lcdStateChange = 1;

//Matrix pins
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 8;
const int rows = 8;
const int cols = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER
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
const int nrOptions = 4;
String menuOptions[] = {"Start Game", "High Scores", "Settings", "Change Name"}; // optiunile de selectie din meniu
const int startGameOption = 0;
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

//Setings screee
int curSetting = 0;
int nrOfSettings = 3;
bool soundOn = 1;
int soundAddress = 70;//EEPROM address
String settingsMenu[] = {"Sound", "Contrast", "Brightness"};
const int settingsOption = 2;
const int changeNameOption = 3;
int nextSetting;

//joystick pins
const int pinSW = 0;
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
int stage = 1;
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
const int shotDelay = 30;
long lastShotMove = 0;
long gameplayStartTime = 0;
long gameplayTimeLimit = 10000;

//eeprom and high scores
const int nrOfHighScores = 3;
int highScoresOffset = 2; // scorurile se afla pe pozitiile 0, 2 si 4
int highScoresNamesOffset = 10; // numele incep de pe pozitiile 10, 20 si 30
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
 
void setup()
{
  lcd.createChar (1, upArrow);
  lcd.createChar (2, downArrow);
  lcd.createChar (3, rightArrow);
  lcd.createChar (4, leftArrow);
  lcd.createChar (5, heartHollow);
  lcd.createChar (6, heartFilled);
  lcd.begin(16,2);
  lcd.setCursor(0, 0);
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
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
      curOption += 1;
      if(curOption == nrOptions)
        curOption = 0;
      lcdStateChange = 1;
    }
    if(joyUp == true){
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

void startGame(){
  score = 0;
  lc.setLed(0, playerRow, playerPos, true); // turns on LED at col, row
  stage = gameplayStage;
  gameplayStartTime = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.setCursor(13, 0);
  for(int i = 0; i < 3; i++){
    lcd.print ((char) 0x06);
  }
  lcd.setCursor(0, 1);
  lcd.print("Score:");
  lcd.setCursor(10, 1);
  lcd.print(score);
}


void downloadHighScores(){
  for(int i = 0; i < nrOfHighScores; i++){
    highScores[i] = EEPROM.read(i * highScoresOffset);
    Serial.print(i);
    Serial.print(") score: ");
    Serial.print(highScores[i]);
    Serial.print(", len: ");
    int hsNameLen = EEPROM.read((i + 1) * highScoresNamesOffset);
    Serial.print(hsNameLen);
    Serial.print(", name: ");
    char hsChar;
    String hsName = "";
    for(int j = 0; j < hsNameLen; j++){
      hsChar = EEPROM.read((i + 1) * highScoresNamesOffset + 1 + j);
      hsName = hsName + hsChar;
    }
    highScoresNames[i] = hsName;
    Serial.print(hsName);
    Serial.println("");
  }
}

void downloadSettings(){
  contrastLevel = EEPROM.read(contrastAddress);
  brightnessLevel = EEPROM.read(brightnessAddress);
  soundOn = EEPROM.read(soundAddress);
  analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
  analogWrite(brightnessPin, brightnessOffset + brightnessMult * brightnessLevel);
}

void uploadSettings(){
  EEPROM.write(contrastAddress, contrastLevel);
  EEPROM.write(brightnessAddress, brightnessLevel);
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
      EEPROM.write(i * highScoresOffset, highScores[i]);
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
      if(c == 32 || c == 65){ //ASCII code for space
        c = 90; //ASCII code for Z
      }
      else
        c --;
    }
    if(joyUp == true){
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
    highScoreInFocus --;
    lcdStateChange = 1;
  }
  if(joyDown && highScoreInFocus < nrOfHighScores - 2){
    highScoreInFocus ++;
    lcdStateChange = 1;
  }
}

void joyStickListener(){
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  switchState = digitalRead(pinSW);
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
  // button
  if(switchState == HIGH && lastSwitchState == LOW){
    clicked = true;
  }
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
  if(clicked){
    stage = gameMenuStage;
    lcdStateChange = 1;
  }
}

void setingsScreen(){
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
  if(joyUp && curSetting > 0){
    if(curSetting == 0)
      curSetting = nrOfSettings - 1;
    else
      curSetting--;
    lcdStateChange = 1;
  }
  if(joyDown){
    curSetting = nextSetting;
    lcdStateChange = 1;
  }
  if(joyRight){
    if(curSetting == 0){
      soundOn = !soundOn;
    }
    if(curSetting == 1 && contrastLevel < 10){
      contrastLevel ++;
      analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
    }
    if(curSetting == 2 && brightnessLevel < 10){
      brightnessLevel ++;
      analogWrite(brightnessPin, brightnessOffset + brightnessMult * brightnessLevel);
    }
    lcdStateChange = 1;
  }
  if(joyLeft){
    if(curSetting == 0){
      soundOn = !soundOn;
    }
    if(curSetting == 1 && contrastLevel > 0){
      contrastLevel --;
      analogWrite(contrastPin, contrastOffset + contrastMult * contrastLevel);
    }
    if(curSetting == 2 && brightnessLevel > 0){
      brightnessLevel --;
      analogWrite(brightnessPin, brightnessOffset + brightnessMult * brightnessLevel);
    }
    lcdStateChange = 1;
  }
}

int settingValue(int settingIndex){
    if(settingIndex == 1)
        return contrastLevel;
    if(settingIndex == 2)
        return brightnessLevel;
}

void loop(){
  joyStickListener();
  if(shotActive){
    if(millis() - lastShotMove > shotDelay){
      lastShotMove = millis();
      lc.setLed(0, shotRow, shotCol, false);
      shotRow -= 1;
      if(shotRow >= 0)
        lc.setLed(0, shotRow, shotCol, true);
      else
        shotActive = 0;
    }
  }
  if(!highScoresInitialized){
    downloadHighScores();
    downloadSettings();
    highScoresInitialized = 1;
  }
  if(stage == welcomeScreenStage){
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
      clicked = false; // set clicked to false in case the button was pressed before the animation finished 
    }
    if(clicked){
      clicked = false;
      lcd.clear();
      stage = enterNameStage1;
    }
  }
  if(stage == enterNameStage1){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter name:");
    lcd.setCursor(curNamePos + 1, 1);
    lcd.print((char) 0x04);
    lcd.setCursor(curNamePos, 1);
    lcd.print(curNameChar);
    stage = enterNameStage2;
  }
  if(stage == enterNameStage2){
    changeNameChar();
    if(clicked){
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
  if(stage == gameplayStage){
    if(millis() - gameplayStartTime > gameplayTimeLimit){
      stage = gameOverStage;
      lcdStateChange = 1;
      uploadScore();
      //score = 0;
    }
    
    lcd.setCursor(5, 0);
    int timeLeft = (gameplayTimeLimit - (millis() - gameplayStartTime)) / 100;
    lcd.print(timeLeft/10);
    lcd.print(".");
    if(timeLeft%10%3 == 0 || timeLeft%10%5 == 0)
      lcd.print(timeLeft%10);
    
    movePlayer();
    if(clicked && !shotActive){
      clicked = false;
      shotActive = 1;
      shotRow = 6; // the row the shot appears on;
      score++;
      lcd.setCursor(10, 1);
      lcd.print(score);
      lastShotMove = millis();
      shotCol = playerPos;
      lc.setLed(0, shotRow, shotCol, true);
    }
  }
  if(stage == gameMenuStage){
    for(int i = 0; i < nrOfHighScores; i++){
      Serial.print(i);
      Serial.print(") score: ");
      Serial.print(highScores[i]);
      Serial.print(", name: ");
      Serial.print(highScoresNames[i]);
      Serial.println("");
    }
    if(lcdStateChange){
      printStartMenu();
      lcdStateChange = 0;
    }
    if(clicked){
      clicked = false;
      if(curOption == startGameOption){
        startGame();
      }
      if(curOption == highScoresOption){
        lcdStateChange = 1;
        stage = highScoreStage;
        highScoreInFocus = 0;
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
    }
    changeOption();
  }
  if(stage == highScoreStage){
    showHighScores();
    if(clicked){
      stage = gameMenuStage;
      lcdStateChange = 1;
    }
  }
  if(stage == gameOverStage){
    gameOverScreen();
  }
  if(stage == settingStage){
    setingsScreen();
    if(clicked){
      lcdStateChange = 1;
      uploadSettings();
      stage = gameMenuStage;
    }
  }
  joyUp = false;
  joyDown = false;
  joyLeft = false;
  joyRight = false;
  clicked = false;
}
