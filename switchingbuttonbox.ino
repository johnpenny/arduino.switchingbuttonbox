//░ 🛈 ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
//░ Author: John Penny © 2020; All rights reserved
//░ License: Dictated by project root license
//░ Contact: JohnPenny+work@gmail.com
//░ Notes: 

#include <Joystick.h> // Joystick Library -- https://github.com/MHeironimus/ArduinoJoystickLibrary
#include <HID-Project.h> // HID-Project -- https://github.com/NicoHood/HID
#include <HID-Settings.h>

namespace JohnPenny
{
	namespace Arduino
	{
		namespace ButtonBox
		{
			enum class OutputMode // we will only have two modes for this button box, media (consumer) and game (keyboard) modes
			{
				UNSET = 0,
				MEDIA = 1, // consumer media keys such as volume and music controls
				GAME = 2, // an emulated USB joystick with buttons and axes
			};

			enum class PinType
			{
				UNSET = 0,
				DIGITAL = 1,
				ANALOGUE = 2
			};

			enum class ControlType
			{
				UNSET = 0,
				BUTTON = 1, // momentary contact button
				SWITCH = 2, // state keeping switch
				RENCODER = 3, // rotary encoder (digital directional click dial)
				POT = 4, // potentiometer (analogue dial with a min & max)
				STICK = 5, //
				BALL = 6, //
				VSWITCH = 7, // VIRTUAL switch from a momentary contact button -- the switching state will be held in sofware -- all modes
				VSWITCH_G = 8, // VIRTUAL switch only in game mode -- falls back to regular button in other modes
				VSWITCH_M = 9, // VIRTUAL switch only in media mode -- falls back to regular button in other modes
			};

			struct Control
			{
				uint8_t id; // a unique ID for error handling (currently just the array index, ie the number at which the pin was set up)
				bool active; // a status bool as we cannot use non assignment safely for status
				bool held; // was this control being held on in the last loop? (NOTE: new explicit hold field rather than using timestamp 0)
				uint16_t timestamp; // a timestamp to rate limit button handling via the interval && mitigate input bounce -- ie when a button is held re-send input every N ms and when an input is pressed there must be N ms before another of the same can register
				uint16_t inputIntervalMillis; // the interval at which some inputs will repeat when the control is held (in ms)
				uint16_t bounceMitigationMillis; // the time in ms after which to allow the control to be used again
				PinType pinType; // the pin read method we are going to use
				ControlType controlType; // the type of physical control
				uint8_t joystickKeycode; // the keycode to use while in the joystick (game pad) mode
				ConsumerKeycode consumerKeycode; // the keycode to use while in the consumer (media key) mode

				uint8_t pin; // the digital pin assignment from the board schematic

				Control()
				{
					this->id = 0u;
					this->active = false;
					this->held = false;
					this->timestamp = 99999u;
					this->inputIntervalMillis = 1000u;
					this->bounceMitigationMillis = 30u;
					this->pinType = PinType::UNSET;
					this->controlType = ControlType::UNSET;
					this->joystickKeycode = 0u;
					this->consumerKeycode = HID_CONSUMER_UNASSIGNED;

					this->pin = 0u;
				}
			};

			class ButtonBox
			{
			public:

				ButtonBox()
				{
					controls = new Control[pinCount]{};
				}

				bool bounceMitigation = true; // try to mitigate input bounce which is a common issue with basic momentary buttons (the input goes on off on, not clean)
				bool debug = false; // !IMPORTANT this will HALT until you open a serial connection, you must understand that this will break the plug and play function of the board!
				bool isGameMode; // cache the mode state (more complex mode select is unrequired right now) allows quick bool invert
				uint8_t modePin = 255; // pin to use for mode
				uint16_t aRateLimit = 1000u; // analogue read master interval
				uint8_t pinCount = 20u; // 20 available pins on sparkfun pro micro -- 9 A pins and 20 D pins (overlapped in function)
				uint8_t ledInput = 17; // the LED to flash when an input is detected
				uint8_t feedbackMediaMode = 10; // the pin to hold LOW while in media mode
				uint8_t feedbackGameMode = 16; // the pin to hold LOW while in game mode

				/////////

				void Setup()
				{
					if (debug)
					{
						Serial.begin(9600);
						while (!Serial); // await serial connection
					}

					// check if game mode, otherwise we will just default to media mode (simple 2 mode system right now)
					outputMode = (isGameMode) ? OutputMode::GAME : OutputMode::MEDIA;

					// control list mgmt
					activeControls = new Control[activeControlCount]{}; // pare down the list of controls from the potential max to only active pins
					for (int i = 0; i < activeControlCount; i++) // grab the active controls from the front of the array
					{
						activeControls[i] = controls[i];
					}

					if (debug)
					{
						Serial.print("ACTIVE PIN COUNT: ");
						Serial.print(activeControlCount);
						Serial.print("\n");
					}

					delete[] controls;

					// feedback
					pinMode(ledInput, OUTPUT);
					pinMode(feedbackMediaMode, OUTPUT);
					pinMode(feedbackGameMode, OUTPUT);

					// feedback states
					digitalWrite(ledInput, LOW);

					if (isGameMode)
					{
						digitalWrite(feedbackGameMode, HIGH);
						digitalWrite(feedbackMediaMode, LOW);
					}
					else
					{
						digitalWrite(feedbackMediaMode, HIGH);
						digitalWrite(feedbackGameMode, LOW);
					}

					// HID-Project init
					Consumer.begin();
					BootKeyboard.begin();

					// Joystick Library init
					Joystick.begin();

					// RENCODER TESTING
					/*pinMode(encoderPinA, INPUT);
					pinMode(encoderPinB, INPUT);
					digitalWrite(encoderPinA, HIGH);
					digitalWrite(encoderPinB, HIGH);*/
					// E RENCODER TESTING
				}

				void Loop()
				{
					// check if game mode, otherwise we will just default to media mode (simple 2 mode system right now)
					outputMode = (isGameMode) ? OutputMode::GAME : OutputMode::MEDIA;

					time = millis();

					for (int i = 0; i < activeControlCount; i++) // polling active controls
					{
						HandleControl(activeControls[i], debug);
					}

					// RENCODER TESTING
					//bool encoderA = digitalRead(encoderPinA);

					//if ((encoderALast == HIGH) && (encoderA == LOW))
					//{
					//	if (digitalRead(encoderPinB) == LOW)
					//	{
					//		encoderPos--;
					//		Serial.print("RENCODER LEFT \n");
					//	}
					//	else
					//	{
					//		encoderPos++;
					//		Serial.print("RENCODER RIGHT \n");
					//	}
					//	/*angle = (encoderPos % encoderStepsPerRevolution) * 360 / encoderStepsPerRevolution;
					//	Serial.print("RENCODER ");
					//	Serial.print(encoderPos);
					//	Serial.print(" ");
					//	Serial.println(angle);
					//	Serial.print("\n");*/
					//}
					//encoderALast = encoderA;
					// E RENCODER TESTING

				}

				/////////

				void PinSetup(uint8_t pin, ControlType controlType = ControlType::UNSET, uint8_t joystickKeycode = 0u, ConsumerKeycode consumerKeycode = (ConsumerKeycode)0u, uint16_t inputIntervalMillis = 1000u, uint16_t bounceMitigationMillis = 30u)
				{
					// infer pintype from control type -- defaulting to digital
					switch (controlType)
					{
					case ControlType::BUTTON:
					case ControlType::SWITCH:
					case ControlType::RENCODER:
					case ControlType::VSWITCH:
					case ControlType::VSWITCH_G:
					case ControlType::VSWITCH_M:
					case ControlType::UNSET:
					{
						controls[activeControlCount].pinType = PinType::DIGITAL;
						break;
					}
					case ControlType::POT:
					case ControlType::STICK:
					case ControlType::BALL:
					{
						controls[activeControlCount].pinType = PinType::ANALOGUE;
						break;
					}
					}

					if (controls[activeControlCount].pinType == PinType::DIGITAL) pinMode(pin, INPUT_PULLUP);

					controls[activeControlCount].active = true;
					controls[activeControlCount].id = activeControlCount;
					controls[activeControlCount].pin = pin;
					controls[activeControlCount].controlType = controlType;
					controls[activeControlCount].joystickKeycode = joystickKeycode;
					controls[activeControlCount].consumerKeycode = consumerKeycode;
					controls[activeControlCount].inputIntervalMillis = inputIntervalMillis;
					controls[activeControlCount].bounceMitigationMillis = bounceMitigationMillis;
					activeControlCount++;
				}

			private:

				Joystick_ Joystick;

				OutputMode outputMode = OutputMode::UNSET; // this enum just provides a nicer way to read the control input flow, is synched with game mode bool

				Control* controls; // all potential control pin slots, limited by pin count - used during setup
				Control* activeControls; // a pared down array of only the configured controls - used when polling

				uint8_t activeControlCount; // the number of active controls
				uint16_t time; // updated with the current millis time each loop
				uint16_t aRateLimitTimestamp;
				bool joybuttons[32]; // state for joy lib buttons (allows simulation of latched switched for momentary switches)
				int state; // input handling state cache

				// RENCODER TESTING
				//const static uint8_t encoderPinA = 21u;
				//const static uint8_t encoderPinB = 20u;
				//uint16_t angle = 0u;
				//uint8_t encoderPos = 0u;
				//uint8_t encoderStepsPerRevolution = 23u;
				//boolean encoderALast = LOW; //remembers the previous state of the encoder pin A
				// E RENCODER TESTING

				/////////

				void HandleControl(Control &control, bool serialDebug) // polls controls and then usually transmits some input via USB, also allows serial debug
				{
					// mode switcher - special case
					if (control.pin == modePin)
					{
						if (digitalRead(control.pin) == LOW)
						{
							// bounce mitigation
							if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;
							
							// still held - do nothing
							if (control.held == true) return;

							// timestamp this event
							control.timestamp = time;

							// state
							control.held = true;

							// switch modes
							isGameMode = !isGameMode;

							// feedback
							if (isGameMode)
							{
								digitalWrite(feedbackGameMode, HIGH);
								digitalWrite(feedbackMediaMode, LOW);
							}
							else
							{
								digitalWrite(feedbackMediaMode, HIGH);
								digitalWrite(feedbackGameMode, LOW);
							}

							if (serialDebug)
							{
								Serial.print(time);
								Serial.print("  GAME MODE SWITCHED TO: ");
								Serial.print(isGameMode);
								Serial.print("\n");
							}
						}
						else
						{
							// bounce mitigation
							if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

							// timestamp this event
							control.timestamp = time;

							// state
							control.held = false;
						}

						return; // return early
					}

					switch (control.pinType)
					{
					case PinType::DIGITAL:
					{
						state = digitalRead(control.pin);

						if (state == LOW && serialDebug && control.held == false) // mitigate with hold or this floods and lags the reporting
						{
							Serial.print(time);
							Serial.print("  Digital INPUT DPIN: ");
							Serial.print(control.pin);
							Serial.print("\n");
						}

						switch (outputMode)
						{
						case OutputMode::MEDIA:
						{
							switch (control.controlType)
							{
							case ControlType::VSWITCH_G: // a game only vswitch falls back to button
							case ControlType::BUTTON:
							{
								switch (state)
								{
								case LOW: // LOW pulled down (ON) (pressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// OPTION: still held - do nothing
									//if (control.held == true) return;

									// OPTION: still held - wait until the interval has elapsed before allowing another input
									if (control.held == true && time - control.timestamp < control.inputIntervalMillis) return;

									// timestamp this event
									control.timestamp = time;

									// state
									control.held = true;

									// feedback (LED)
									digitalWrite(ledInput, HIGH);

									// soft input changes
									Consumer.write(control.consumerKeycode);

									// finish
									break;
								}
								case HIGH: // HIGH pulled up (OFF) (unpressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// state
									control.held = false;

									// timestamp this event
									control.timestamp = time;

									// feedback (LED)
									digitalWrite(ledInput, LOW);

									// soft input changes
									// media buttons are one shot inputs

									//finish
									break;
								}
								}
								break;
							}
							case ControlType::VSWITCH_M:
							case ControlType::VSWITCH:
							{
								// i currently cannot find a functional purpose for a media switch
								if (serialDebug)
								{
									Serial.print(time);
									Serial.print("  MEDIA VSWITCH UNSUPPORTED ");
									Serial.print(control.id);
									Serial.print("\n");
								}
								break;
							}
							case ControlType::UNSET:
							{
								if (serialDebug)
								{
									Serial.print(time);
									Serial.print("  CONTROL TYPE WAS UNSET ");
									Serial.print(control.id);
									Serial.print("\n");
								}
								break;
							}
							}
							break;
						}
						case OutputMode::GAME:
						{
							switch (control.controlType)
							{
							case ControlType::VSWITCH_M: // a media only vswitch falls back to button
							case ControlType::BUTTON:
							{
								switch (state)
								{
								case LOW: // LOW pulled down (ON) (pressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// OPTION: still held - do nothing
									if (control.held == true) return;

									// OPTION: still held - wait until the interval has elapsed before allowing another input
									//if (control.held == true && time - control.timestamp < control.inputIntervalMillis) return;

									// timestamp this event
									control.timestamp = time;

									// state
									control.held = true;

									// feedback (LED)
									digitalWrite(ledInput, HIGH);

									// soft input changes
									Joystick.setButton(control.joystickKeycode, true); // set on

									// finish
									break;
								}
								case HIGH: // HIGH pulled up (OFF) (unpressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// state
									control.held = false;

									// timestamp this event
									control.timestamp = time;

									// feedback (LED)
									digitalWrite(ledInput, LOW);

									// soft input changes
									Joystick.setButton(control.joystickKeycode, false); // set off - no longer being held

									//finish
									break;
								}
								}
								break;
							}
							case ControlType::VSWITCH_G:
							case ControlType::VSWITCH:
							{
								switch (state)
								{
								case LOW: // LOW pulled down (ON) (pressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// OPTION: still held - do nothing
									if (control.held == true) return;

									// OPTION: still held - wait until the interval has elapsed before allowing another input
									//if (control.held == true && time - control.timestamp < control.inputIntervalMillis) return;

									// timestamp this event
									control.timestamp = time;

									// state
									control.held = true;

									// feedback (LED)
									digitalWrite(ledInput, HIGH);

									// soft input changes
									joybuttons[control.joystickKeycode] = !joybuttons[control.joystickKeycode]; // virtual switch (software state) switcher
									Joystick.setButton(control.joystickKeycode, joybuttons[control.joystickKeycode]); // set with new inverted value

									// finish
									break;
								}
								case HIGH: // HIGH pulled up (OFF) (unpressed)
								{
									// bounce mitigation
									if (bounceMitigation && time - control.timestamp < control.bounceMitigationMillis) return;

									// state
									control.held = false;

									// timestamp this event
									control.timestamp = time;

									// feedback (LED)
									digitalWrite(ledInput, LOW);

									// soft input changes
									// off states are handled by the virtual switch state (each discrete press just inverts a bool)

									//finish
									break;
								}
								}
								break;
							}
							case ControlType::UNSET:
							{
								if (serialDebug)
								{
									Serial.print(time);
									Serial.print("  CONTROL TYPE WAS UNSET ");
									Serial.print(control.id);
									Serial.print("\n");
								}
								break;
							}
							}
							break;
						}
						case OutputMode::UNSET:
						{
							if (serialDebug)
							{
								Serial.print(time);
								Serial.print("  OUTPUT MODE WAS UNSET ");
								Serial.print(control.id);
								Serial.print("\n");
							}
							break;
						}
						}
						break;
					}
					case PinType::ANALOGUE:
					{
						if (time - aRateLimitTimestamp < aRateLimit) return; // too soon

						aRateLimitTimestamp = time; // now

						state = analogRead(control.pin);

						if (serialDebug)
						{
							Serial.print(time);
							Serial.print("  Analogue READ ");
							Serial.print(state);
							Serial.print("\n");
						}
						break;
					}
					case PinType::UNSET:
					{
						if (serialDebug)
						{
							Serial.print(time);
							Serial.print("  PIN TYPE WAS UNSET ");
							Serial.print(control.id);
							Serial.print("\n");
						}
						break;
					}
					}
				}

			};
		}
	}
}

	////////////////////////////////////////////////////////////////////////////

	using namespace JohnPenny::Arduino::ButtonBox;

	ButtonBox buttonBox;

	void setup()
	{
		SetupButtonBox();
	}

	void loop()
	{
		buttonBox.Loop();
	}

	void SetupButtonBox()
	{
		// settings
		buttonBox.debug = false; // this breaks the plug and play aspect of the mcu - must connect to serial to work
		buttonBox.isGameMode = false; // starting mode
		buttonBox.modePin = 3;
		buttonBox.ledInput = 17;
		buttonBox.feedbackMediaMode = 10;
		buttonBox.feedbackGameMode = 16;

		// mode switch pin setup
		buttonBox.PinSetup(3u);

		// pin setup (make sure to double check the schematic for the pin assignment as there is no enforcement of valid pin types or using too many pins)
		buttonBox.PinSetup(9u, ControlType::VSWITCH_G, 0u, HID_CONSUMER_VOLUME_INCREMENT);
		buttonBox.PinSetup(8u, ControlType::VSWITCH_G, 1u, HID_CONSUMER_VOLUME_DECREMENT);
		buttonBox.PinSetup(7u, ControlType::VSWITCH_G, 2u, HID_CONSUMER_MUTE);
		buttonBox.PinSetup(6u, ControlType::VSWITCH_G, 3u, HID_CONSUMER_PLAY_SLASH_PAUSE);
		buttonBox.PinSetup(5u, ControlType::VSWITCH_G, 4u, HID_CONSUMER_SCAN_NEXT_TRACK);
		buttonBox.PinSetup(4u, ControlType::VSWITCH_G, 5u, HID_CONSUMER_SCAN_PREVIOUS_TRACK);

		// run setup
		buttonBox.Setup();
	}
