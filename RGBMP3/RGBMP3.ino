/*
  RGB MP3
  an implementation of my MP3 player with added support for an RGB LED
  Using DFPlayerMini_Fast, FireTimer, EEPROM, RGB and OneButton libraries
  Written by Ashley Cox, https://ashleycox.co.uk
  Code provided as-is with no warranty or suitability implied. May not be reproduced in full or in part without credit unless written permission is obtained
  Libraries the property of their respective creators
*/

//Include libraries
#include <EEPROM.h> //Stores and recalls user-defined values including thevolume
#include <OneButton.h> //handles button events
#include <DFPlayerMini_Fast.h> //Library to control the DFPlayer module
#include <FireTimer.h> //Simple non-blocking time-based event triggering
#include <SoftwareSerial.h> //For software serial communications with the DFPlayer module
#include <RGBLed.h> //Controls the RGB LED

//declare some global variables

//Boolean values, true or false
bool ROM = false; //Is there data in theEEPROM?
bool changeVolume; //Are we in the midst of changing the volume?
bool changeBrightness; //are we in the midst of changing the LED brightness?
bool isPlaying; //The DFPlayer's own status reports are unreliable, so we keep track of the status ourselves

//Integers
int currentVolume; //Set based on a value stored in EEPROM, or defaults to 10
int volumeDelay = 500; //the speed in milliseconds at which the volume ramps up or down. Also used when changing the LED brightness.
int ledDelay = 75; //the speed at which the LED brightness increments or decrements
int EEAddress = 0; //Current address of EEPROM data being accessed or written
int ledBrightness; //LED brightness
int ledFadeTime; //The duration of a fade in or out
int ledFadeSteps = 10; //determines the number of steps in a fade. Higher numbers give a smoother transition
int ledState = 0; //used for colour toggling
int colourChangeDuration = 5000; //how often to change colours and how long to wait before saving the colour to the EEPROM to avoid too many writes

//LED colours
//These are predefined colours supported by theRGB library
//Additional colours can be added by referencing their RGB colour codes
char *ledColours[] = {"RGBLed::RED", "RGBLed::GREEN", "RGBLed::BLUE", "RGBLed::MAGENTA", "RGBLed::CYAN", "RGBLed::YELLOW", "RGBLed::WHITE"};
char currentColour; //records the current colour

//Setup library objects
//Software Serial on pins 7 (RX) and 8 (TX) leaves pins 9, 10 and 11 free for an RGB LED
SoftwareSerial mySerial(7, 8);
//instantiate the DFPlayer library
DFPlayerMini_Fast mp3;
// Setup a button on d§igital pin D4
OneButton button1(4, true);
// Setup a button on digital pin D5
OneButton button2(5, true);
//Setup a button on digital pin D6
OneButton button3(6, true);
FireTimer volRampDelay;
FireTimer ledRampDelay;
FireTimer colourTimer;

//Setup the RGB LED on Pin 11 (Red), pin 10 (Green), pin 9 (blue).
//ANy PWM pins can be used
RGBLed led(11, 10, 9, RGBLed::COMMON_CATHODE);

void setup() {
  //This code runs once

  //First check the EEPROM
  if (!EEPROM.get(EEAddress, ROM)) {
    //No settings have been written to the EEPROM yet
    //Set some default values
    currentVolume = 10; //A sensible starting value with most speakers
    currentColour = "RGBLed::BLUE"; //Every electronic project should have at least one blue LED, apparently
    ledBrightness = 100; // full brightness
    ledFadeTime = 200; //200 milliseconds
  }
  else {
    //There are settings stored in ROM
    ROM = true;
    //Find the first EEPROM address
    EEAddress += sizeof(ROM);
    //Get the last volume state saved in the EEPROM
    currentVolume = EEPROM.get(EEAddress, currentVolume);
    //find the last colour
    EEAddress += sizeof(currentVolume); //Increment the EEPROM address
    currentColour = EEPROM.get(EEAddress, currentColour);
    //find the stored LED brightness
    EEAddress += sizeof(currentColour); //Increment the EEPROM address
    ledBrightness = EEPROM.get(EEAddress, ledBrightness);
    //we also stored the LED fade time
    EEAddress += sizeof(ledBrightness); //Increment the EEPROM address
    ledFadeTime = EEPROM.get(EEAddress, ledFadeTime);
  } //End if

  //fade in the LED
//we don't want to fade to full brightness, so we set the brightness in an off state
led.brightness(ledBrightness); 
  led.fadeIn(currentColour, ledFadeSteps, ledFadeTime);

  //Setup a serial monitor so we can see what the program is doing
  Serial.begin(9400);
  //setup software serial for DFPlayer communication
  mySerial.begin(9600);

  //initialise the DFPlayer library using the software serial connection, with debug output enabled
  mp3.begin(mySerial, true);
  //Set theDFPlayer volume, or it will startup at full volume by default
  Serial.println("Setting volume to ");
  Serial.print(currentVolume);
  mp3.volume(currentVolume);

  // link the button 1 functions.
  Serial.println("Setting up button 1");
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleClick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  // link the button 2 functions.
  Serial.println("Setting up button 2");
  button2.attachClick(click2);
  button2.attachDoubleClick(doubleClick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);

  // link the button 3 functions.
  Serial.println("Setting up button 3");
  button3.attachClick(click3);
  button3.attachDoubleClick(doubleClick3);
  button3.attachLongPressStart(longPressStart3);
  button3.attachLongPressStop(longPressStop3);
  button3.attachDuringLongPress(longPress3);

  //set up a timer for the volume ramp
  volRampDelay.begin(volumeDelay);
  //setup a timer for LED brightness changes
  ledRampDelay.begin(ledDelay);
  //setup a timer for LED colour changes
  colourTimer.begin(colourChangeDuration);

  //let the DFPlayer catch up
  delay(1000);

  //check if we're playing, in case the arduino crashed
  if (mp3.isPlaying()) {
Serial.println("Music is already playing on startup");
    isPlaying = true;
  } else {
    Serial.println("No music is playing at startup. Playing a random track");
    isPlaying = true;
    //play something random at startup
    mp3.randomAll();
  } //end if
} //End setup

void loop() {
  //this code loops repeatedly

  // keep watching the push buttons:
  button1.tick();
  button2.tick();
  button3.tick();

  delay(20); //Blocking delay to stop the program tripping over itself
} //End loop

// This function will be called when button1 ispressed once
void click1() {
  Serial.println("Button 1 click.");
  if (isPlaying) {
    mp3.playNext();
    delay(20);
  } else {
    mp3.playFolder(1, 1);
    isPlaying = true;
  } //end if
} //end function

// This function will be called when button1 ispressed twice in a short timeframe
void doubleClick1() {
  Serial.println("Button 1 doubleclick.");
  if (isPlaying) {
    mp3.playPrevious();
    delay(20);
  } else {
    isPlaying = true;
    mp3.randomAll();
  } //end if
} //end function

// This function will be called once, when button1 is held
void longPressStart1() {
  Serial.println("Button 1 longPress start");
  //if we're playing music, change thevolume. Otherwise change the brightness
  if (isPlaying) {
    //We're going to change the volume
    changeVolume = true;
  } else {
    //we'll change the brightness
    changeBrightness = true;
  } //end if
} //end function

// This function will be called often, when button1 is held
void longPress1() {
  Serial.println("Button 1 longPress...");
  if (isPlaying) {
    //If we're changing the volume, and the current volume is more than theminimum possible
    if (changeVolume && currentVolume > 0) {
      //decrement the current volume by 1
      if (volRampDelay.fire()) {
        currentVolume --;
        //set the DFPlayer to thenew volume level
        mp3.volume(currentVolume);
      } //end if
    } //end if
  } else {
    if (changeBrightness && ledBrightness > 1) {
      //decrement the current brightness level by 1
      if (ledRampDelay.fire()) {
        ledBrightness --;
        //set the LED to thenew brightness level
        led.brightness(currentColour, ledBrightness);
      } //end if
    } //end if
  } //end if
} //end function

// This function will be called once, when button1 is released after a long press
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
  if (isPlaying) {
    //store the final volume value in EEPROM
    EEAddress = 0 + sizeof(ROM); // we need to work out the EEPROM address to store the updated value
    EEPROM.put(EEAddress, currentVolume); //Store the updated value in theEEPROM. The value is only updated if it changes
    //We're no-longer changing thevolume
    changeVolume = false;
  } else {
    //we've finished changing the brightness, so we'll store the value in EEPROM
    EEAddress = 0 + sizeof(ROM) + sizeof(currentVolume) + sizeof(currentColour); // we need to work out the EEPROM address to store the updated value
    EEPROM.put(EEAddress, ledBrightness); //Store the updated value in theEEPROM. The value is only updated if it changes
    changeBrightness = false;
  } //end if
} //end function

//The other button functions are as above

void click2() {
  Serial.println("Button 2 click.");
  if (isPlaying) {
    isPlaying = false;
    mp3.pause();
  } else {
    isPlaying = true;
    mp3.resume();
  } //end if
} //end function

void doubleClick2() {
  Serial.println("Button 2 doubleclick.");
  if (isPlaying) {
    isPlaying = false;
    mp3.stop();
  } else {
    isPlaying = true;
    mp3.randomAll();
  } //end if
} //end function

void longPressStart2() {
  Serial.println("Button 2 longPress start");
  //if we're playing music, change thevolume. Otherwise change the brightness
  if (isPlaying) {
    //We're going to change the volume
    changeVolume = true;
  } else {
    //we'll change the brightness
    changeBrightness = true;
  } //end if
} //end function

void longPress2() {
  Serial.println("Button 2 longPress...");
  if (isPlaying) {
    //If we're changing the volume, and the current volume is less than the maximum possible
    if (changeVolume && currentVolume < 30) {
      //increment the current volume by 1
      if (volRampDelay.fire()) {
        currentVolume ++;
        //set the DFPlayer to thenew volume level
        mp3.volume(currentVolume);
      } //end if
    } //end if
  } else {
    if (changeBrightness && ledBrightness < 100) {
      //increment the current brightness level
      if (ledRampDelay.fire()) {
        ledBrightness ++;
        //set the LED to thenew brightness level
        led.brightness(currentColour, ledBrightness);
      } //end if
    } //end if
  } //end if
} //end function

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
  if (isPlaying) {
    //store the final volume value in EEPROM
    EEAddress = 0 + sizeof(ROM); // we need to work out the EEPROM address to store the updated value
    EEPROM.put(EEAddress, currentVolume); //Store the updated value in theEEPROM. The value is only updated if it changes
    //We're no-longer changing thevolume
    changeVolume = false;
  } else {
    EEAddress = 0 + sizeof(ROM) + sizeof(currentVolume) + sizeof(currentColour); // we need to work out the EEPROM address to store the updated value
    EEPROM.put(EEAddress, ledBrightness); //Store the updated value in theEEPROM. The value is only updated if it changes
    changeBrightness = false;
  } //end if
} //end function

void click3() {
  Serial.println("Button 3 click");
  if (ledState > 7) {
    ledState = 0;
  }
  currentColour = ledColours[ledState];
  led.brightness(currentColour, ledBrightness);
  ledState ++;
  if (colourTimer.fire()) {
    EEAddress = 0 + sizeof(ROM) + sizeof(currentVolume); // we need to work out the EEPROM address to store the updated value
    EEPROM.put(EEAddress, currentColour); //Store the updated value in theEEPROM. The colour is only saved after a pre-defined time set in colourChangeDuration
  }
} //end function

void doubleClick3() {
  Serial.println("Button 3 doubleClick");
  if (colourTimer.fire()) { //if it's time to change the colour
    ledState = random(0, 7); //choose a random colour
    if (currentColour == ledColours[ledState]) { //if this is already the colour
      ledState = random(0, 7); //choose another random number
    } //end if
    currentColour = ledColours[ledState];
    led.brightness(currentColour, ledBrightness);
  } //end if
} //end function

void longPressStart3() {
  Serial.println("Button 3 longPress start");

} //end function

void longPress3() {
  Serial.println("Button 3 longPress");

} //end function

void longPressStop3() {
  Serial.println("Button 3 longPress stop");

} //end function
