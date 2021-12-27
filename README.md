This project involves creating a arduino-based game using a 8x8 led matrix, a joystick, a LCD display and a buzzer.<br />
I have added a extra button and a green LED. I use the button to turn the sound on/off and the LED as an indicator for the same setting.<br />
In this game, the player has to shoot blocks to fill the voids in the rows that are descending from the top of the screen. When filled, a row disappears and the player's score increases.<br />
The game ends when the rows reach the player or after the player has taken 3 hits from the falling blocks.<br />
As the score increases the player levels up. With every level the game becomes harder by one of the following:
- The rows are generated faster
- The rows are generated with less blocks
- The blocks start falling

The settings menu includes options to change the values for sound, LCD contrast and both the brightness of the LCD and the matrix.<br />
The player can set his/hers name from the main menu at any time. The name will be displayed in the high scores screen.<br />
The menu also allows for deleting the high scores.<br />
There are 3 slots for high scores that are also written in the Arduino's EEPROM memory that alows for them to be saved when the system is turned off. The settings are also saved in EEPROM.<br />

Bugs:<br />
- Sometimes when the game ends the buzzer does not stop, creating a continuous beep.
- After finishing a game and starting a new one, the generated rows are allready filled and can not be destroyed.
<img src="https://user-images.githubusercontent.com/62668963/147181144-2f3b3ba6-11bc-43da-a286-06156813713e.jpg" width = "1000">
<img src="https://user-images.githubusercontent.com/62668963/147181247-266e3c4c-ac7b-4b39-896c-5a77628fafd5.jpg" width = "1000">
Video: https://youtu.be/hx4l_wuwBdE

&nbsp;&nbsp;&nbsp;&nbsp;loop function:<br />
The functionality is divided in stages. Each stage dictates a different part of the game/menu.<br />
I call a function inside the loop function to register any joystick movement or button press. For each direction (up, down, left, right) and the button I save the coresponding value (true or false) in a variable.<br />
At the end of the loop function, I set every one of those variables to false, so the value does not translate into the next call.<br />
&nbsp;&nbsp;&nbsp;&nbsp;Scrollable menus:<br />
I save the current option (the one in focus) is a variable and I print that one on the first line of the LCD and the next one on the second line.<br />
If I register a joystick movement (joyUp or joyDown are true) I increase or decrease the option by 1 and print again.<br />
I use lcdStateChange variable to decide when to print.<br />
&nbsp;&nbsp;&nbsp;&nbsp;Main menu:<br />
When a click is registered I switch to the stage coresponding to the current option.<br />
&nbsp;&nbsp;&nbsp;&nbsp;Settings:<br />
If joyLeft or joyRight are true I change the value of the setting in focus by 1.<br />
The sound setting can also be changed from the hardware button.<br />
When a click is registered I switch back to the main menu.<br />
&nbsp;&nbsp;&nbsp;&nbsp;EEPROM:<br />
For each value that is saved in EEPROM, I assigned a fixed address.<br />
For the names, I save the length of the name on the assigned address and each character on the next ones.<br />
I read the values at the start of the program and write them each time a change has been made.<br />
&nbsp;&nbsp;&nbsp;&nbsp;Gameplay:<br />
The player is fixed to the bottom row of the matrix. joyLeft and joyRight move the player.<br />
Rows of random positioned blocks are beeing generated and set at the top of the screen. Each row generated pushes the other ones down one row.<br />
When a click is registered the shotActive variable is set to true, the landing position of the shot is calculated and the shot's position is set right over the player.<br />
While shotActive is true the shot is moved, one row at a time, on the same column, until it reaches it's landing position and shotActive is set to false.<br />
After a certain level blocks from the lowest row start falling. The mechanic is similar to shooting, just moving towards the player.<br />
If the player gets hit by the blocks he loses a life.<br />
When the row is full it disappears and all rows that were lower are moved up.<br />

