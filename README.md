# RGB MP3
 An Arduino-based MP3 player with added support for an RGB LED

This uses code from my [Arduino MP3 player](https://github.com/ashleycox/Arduino-MP3-Player) project, and adds support for an RGB LED. It was built to be used in an electronic module for a child's Ukulele toy. It uses the excellent [RGBLed](https://github.com/wilmouths/RGBLed) library, the current version of the code requires the improved fork by [HPreston](https://github.com/hpreston/RGBLed) which improves the brightness function.

The code requires the [OneButton](https://github.com/mathertel/OneButton), [DFPlayerMini_Fast](https://github.com/PowerBroker2/DFPlayerMini_Fast), [FireTimer](https://github.com/PowerBroker2/FireTimer) & default EEPROM libraries. Be sure you have those installed before attempting to compile the sketch.

Buttons 1 and 2 primarily control the MP3 playback. The exception is that when no music is playing, a long press on either will alter the LED brightness. Button 3 changes the colour on a single press, and a double press cycles the LED between colours at random. 