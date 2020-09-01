# Arduino Switching USB Button Box
A simple media/game switchable USB button box. Using a Sparkfun Pro Micro.

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
* Wire up two LEDS, one to pin 10, and one to 16
* Wire some momentary buttons up to pins 9,8,7,6,5,4 and GND (ground)
* Wire up a momentary button to pin 3 and GND

##### Customise
The project is set up so that the media keys are timed out (holding causes input interval, must press and release for fast input) buttons for: 4 (prev), 5 (next), 6 (pause/play), 7 (mute), 8 (vol down), 9 (vol up); and in GAME mode all buttons are virtual on/off switches (I'm using it mainly for flight sim) and you can demo functionality via the "USB game controllers" utility in Windows. You may set all the pin numbers and media functions to ones relevant to your setup.

NOTE: The only control types that are coded up are BUTTON and VSWITCH_G, with VSWITCH_G falling back to BUTTON type when in media mode.
