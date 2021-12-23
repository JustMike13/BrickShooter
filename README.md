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
<img src="https://user-images.githubusercontent.com/62668963/147181144-2f3b3ba6-11bc-43da-a286-06156813713e.jpg" width = "1000">
<img src="https://user-images.githubusercontent.com/62668963/147181247-266e3c4c-ac7b-4b39-896c-5a77628fafd5.jpg" width = "1000">
Video: https://youtu.be/hx4l_wuwBdE


