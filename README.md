# Arduino USB Switching Button Box
A simple media/game USB plug and play 'mode switching' button box. The same buttons are used, but the mode is switchable, allowing multiplied control functionality. Using a Sparkfun Pro Micro.

### State
Nebulous. Basic functionality is there. Missing input matrix, so pin to control ratio is currently 1:1.

##### TODO
* Matrix input and virtual pins
* Mode stepping support for 2+ modes (likely with a rotary encoder, meaning giving up 2 pins for mode switcher)
* Keyboard input option for controls (allowing keyboard input instead of media or joystick inputs) (this was removed, but I realise that for games this could be valuable)
* Rotary encoder support for +- controls
* Pot support for gradual controls
* Joystick support

### Why Did You Make It?
This is a new button box concept (for me) with a mode switcher that allows multiplying the inputs via a mode switch (or in future a dial with more than 2 modes). The reason I am creating this specific box is so that I can use a single smaller box for basic gaming functions (MSFS2020) and media keys for my music. All my previous button boxes were exiled from my desk, so I am simply trying to make a better solution that is useful enough to stay on my desk all the time.

### How Do I Use It?
##### Prerequisites
* A Sparkfun Pro Micro (or similar USB plug and play board) with a USB cable
* Some momentary buttons at minimum
* A breadboard or alternative
* Preferably a couple of LEDs and relevant resistors

##### Software
* Grab the Joystick Library within arduino IDE or - https://github.com/MHeironimus/ArduinoJoystickLibrary
* Grab the HID-Project within arduino IDE or - https://github.com/NicoHood/HID
* Grab the .ino file from this repo

##### Hardware
* Wire two LEDS to pins 10 and 16 (using some appropriate resistors if required), to GND (ground)
* Wire some momentary buttons to pins 9,8,7,6,5,4, to GND
* Wire a separate (mode switcher) momentary button to pin 3, to GND

##### Customise
The project is set up so that the media keys are timed out (holding causes input interval, must press and release for fast input) buttons for: 4 (prev), 5 (next), 6 (pause/play), 7 (mute), 8 (vol down), 9 (vol up); and in GAME mode all buttons are virtual on/off switches (I'm using it mainly for flight sim) and you can demo functionality via the "USB game controllers" utility in Windows. You may set all the pin numbers and media functions to ones relevant to your setup.

NOTE: The only control types that are coded up are BUTTON and VSWITCH_G, with VSWITCH_G falling back to BUTTON type when in media mode.


