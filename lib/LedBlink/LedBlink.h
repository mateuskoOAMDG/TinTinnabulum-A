/*
 * Library LedBlink
 *
 * The library allows control of LED blinking according to the specified blinking sequence.
 *
 * file   : LedBlink.h
 * author : m$o (mateusko.oamdg@outlook.com)
 * date   : 2024/10/09
 *
 *                                      
 *
 *   - LED_BLINK_NORMAL_MODE 
 *          The blinking sequence is played once           
 *          blinking sequence array format: 
 *                  (tick1_on, tick1_off, tick2_on, tick2_off, ..., tickN_on, 0)
 *
 *   - LED_BLINK_REPEAT_MODE
 *          The blinking sequence will play the specified number of times
 *          Blinking sequence array format: 
 *                  (LED_BLINK_REPEAT_MODE | repeat_count, tick1_on, tick1_off, tick2_on, tick2_off, ..., tickN_on, tickN_off, 0)
 *            
 *
 *   - LED_BLINK_TIME_MODE 
 *          The blinking sequence will play for the set time             
 *          Blinking sequence array format: 
 *                  (LED_BLINK_TIME_MODE | duration_time_ms, tick1_on, tick1_off, tick2_on, tick2_off, ..., tickN_on, tickN_off, 0)
 *
 *   - LED_BLINK_INFINITY_MODE          
 *          The blinking sequence will play repeatedly indefinitely
 *          Blinking sequence array format: 
 *                  (LED_BLINK_INFINITY_MODE, tick1_on, tick1_off, tick2_on, tick2_off, ..., tickN_on, tickN_off, 0)
 * 
 *
 *          tickn_on          - led is on for ticks_on ticks
 *          tickn_off         - led is offn for ticks_off ticks
 *          0                 - sequence must end with zero (0)
 *          repeat-count      - the number of repetitions of the sequence
 *          duration_time_ms  - sequence replay duration in milliseconds
 */ 


#ifndef _LEDBLINK_H_
#define _LEDBLINK_H_

#define ON  1 // led is ON
#define OFF 0 // led is OFF
#define DEFAULT_LED_TICK_TIME 1 // default tick time in milliseconds

#define LED_BLINK_NORMAL_MODE     0x0000u   // play flashing sequence 1x
#define LED_BLINK_REPEAT_MODE     0x4000u   // play flashing sequence Nx
#define LED_BLINK_TIME_MODE       0x8000u   // play flashing sequence repeatedly for setted time
#define LED_BLINK_INFINITY_MODE   0xC000u   // play flashing sequence repeatedly indefinitely

class LedBlink {
public:

  // constructor
  LedBlink();

  bool isBlinking;  // PROPERTY true - blinking, active | false - not blinking, inactive

  // initializing
  void begin(int pin, int initial_state = OFF);

  // turn LED on
  inline void on() {
    digitalWrite(_pin, HIGH);
    _ledState = ON;
  }

  // turn LED off
  inline void off() {
    digitalWrite(_pin, LOW);
    _ledState = OFF;
  }

  //setting tick-time in milliseconds
  inline void setTickTime(unsigned int tickTime) {
    _tickTime = tickTime;
  }


  // Start blink using command Array with blink pattern
  void blink(unsigned int commandArray[]);
  inline void blink(const unsigned int commandArray[]) {
    blink((unsigned int*) commandArray);
  }

  // Update blink, must by called periodicaly
  void update();

  // Stop blink
  void stop(int state = OFF); 

protected:
  int _pin;       // GPIO pin#
  int _ledState;  // LED state - ON | OFF
  int _index;     // command array item actual index

  unsigned int _tickTime;     //One tick time in milliseconds
  unsigned long _startTimer;  // The Timer for for time-repeat-blinking
  unsigned long _timeTime;    // blinking time in TIME mode
  unsigned long _timer;       // temporary timer
  unsigned long _time;        // actual time in then array item _ca[_index]

  unsigned int* _ca;          // pointer to command-array
  unsigned int _repeat;  // how many the sequence must to repeat
  int _mode;             // 0 - NORMAL | 1 - REPEAT | 2 - TIME Repeat | 3 - INFINITY REPEAT
};













#endif