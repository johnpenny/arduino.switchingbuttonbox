# Arduino USB Switching Button Box
A simple media/game USB switching button box. The same buttons are used, but the mode is switchable, allowing double functionality. Using a Sparkfun Pro Micro.

### State
Nebulous. Basic functionality is there.

### Why?
This is a new button box concept (for me) with a mode switcher that allows multiplying the inputs via a mode switch (or in future a dial with more than 2 modes). The reason I am creating this specific box is so that I can use a single box for basic gaming functions (MSFS2020) and media keys for my music.

### How?
##### Prerequisites
* A Sparkfun Pro Micro (or similar board) with a USB cable
* Some momentary buttons at minimum
* A breadboard or alternative
* Preferably a couple of LEDs and relevant resistors

##### Software
* Grab the Joystick Library within arduino IDE or - https://github.com/MHeironimus/ArduinoJoystickLibrary
* Grab the HID-Project within arduino IDE or - https://github.com/NicoHood/HID
* Grab usbbuttonbox.ino from this repo

##### Hardware
* Wire two LEDS to pins 10 and 16 (using some appropriate resistors if required), to GND (ground)
* Wire some momentary buttons to pins 9,8,7,6,5,4, to GND
* Wire a separate (mode switcher) momentary button to pin 3, to GND

##### Customise
The project is set up so that the media keys are timed out (holding causes input interval, must press and release for fast input) buttons for: 4 (prev), 5 (next), 6 (pause/play), 7 (mute), 8 (vol down), 9 (vol up); and in GAME mode all buttons are virtual on/off switches (I'm using it mainly for flight sim) and you can demo functionality via the "USB game controllers" utility in Windows. You may set all the pin numbers and media functions to ones relevant to your setup.

NOTE: The only control types that are coded up are BUTTON and VSWITCH_G, with VSWITCH_G falling back to BUTTON type when in media mode.
