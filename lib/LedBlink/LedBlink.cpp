/*
 * Library LedBlink
 *
 * The library allows control of LED blinking according to the specified blinking sequence.
 * 
 * file   : LedBlink.cpp
 * author : m$o (mateusko.oamdg@outlook.com)
 * date   : 2024/10/09
 */ 


#include "Arduino.h"
#include "LedBlink.h"

// Constructor
LedBlink::LedBlink() {
  _pin = -1;
  _ledState = OFF;
  isBlinking = false;
}

// Initialize instance
void LedBlink::begin(int pin, int initial_state) {
  _pin = pin;
  _ledState = initial_state;
  _tickTime = DEFAULT_LED_TICK_TIME;
  _mode = 0;  // mode NORMAL

  pinMode(_pin, OUTPUT);

  if (_ledState == ON) {
    on();
  } else {
    off();
  }
}


// Start blink
void LedBlink::blink(unsigned int commandArray[]) {
  int mode;
  int repeat = 0;
  int index;
  mode = commandArray[0] >> 14;  // extract mode from first command in commandArray[]


  switch (mode) {
    case 0:  // NORMAL
      index = 0;
      break;

    case 1:  // REPEAT x Times
      repeat = commandArray[0] & 0x3FFF;
      if (repeat == 0) {
        return;  //nothing action
      }
      index = 1;
      break;

    case 2:  // TIME-REPEAT
      _startTimer = millis();
      index = 1;
      _timeTime = commandArray[0] & 0x3FFF;
      break;

    case 3:  // INFINITY REPEAT
      index = 1;
      break;

    default:
      return;  // unknown command - no action
      break;
  }
  if (commandArray[index] == 0) return; //no command
  if(_tickTime == 0) return; //no command

  _index = index;
  _repeat = repeat;
  _mode = mode;
  _ca = commandArray;
  _time = _ca[_index] * _tickTime;
  if (_time == 0) return;  //no time in command (value 0)

  _ledState = ON;  // turn on led state

  if (_ledState == ON) on();
  else off();

  _timer = millis();
  isBlinking = true;
}

/// @brief Stop blink
/// @param int state ON | OFF
void LedBlink::stop(int state) {
    isBlinking = false;
    if (state == ON) on(); else off();
  }

void LedBlink::update() {
  if (!isBlinking) return;  // not blinking yet
  if (_mode == 2) {
    if (millis() - _startTimer > _timeTime) {
      isBlinking = false;
      off();
    }
  }
  if (millis() - _timer > _time) {
    //read next command
    _index++;

    if (_ca[_index] == 0) {  //end of command sequence
      switch (_mode) {

        case 1:                // REPEAT x Times
          if (_repeat <= 1) {  // end of repeat
            isBlinking = false;
            return;
          } else {
            _repeat--;
            _index = 1;  // start command sequence
          }
          break;

        case 3:     //Infinity repeat
          _index = 1;  // start command sequence
          break;

        case 2:
          _index = 1;
          break;
        case 0:
        
        default:
          if (_ledState == OFF) on();
          else off();  // update LED lighting -toggle

          isBlinking = false;
          return;
      }
    }

    if (_ledState == OFF) on();
    else off();  // update LED lighting -toggle

    _timer = millis();
    _time = _ca[_index] * _tickTime;
  }
}