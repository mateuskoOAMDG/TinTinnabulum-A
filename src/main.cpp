
/// @name TinTinNabulum A.00.03
/// @brief Melodic doorbell with DFPlayer Mini DFR0299 and ATtiny1624 
/// @date 2024/10/31
/// @author m$o, mateusko.oamdg@outlook.com
 

#include "Arduino.h"
#include "EEPROM.h"

// Buttons debouncing and button events library
#include "Button.h"

// Library for controlling LED blinking
#include "LedBlink.h"

// DFPlayer Mini Library
#include "DFRobotDFPlayerMini.h"

// Debug statements to the serial interface
#define DEBUG_ON


// I/O PINS

    #define PIN_LED             PIN_PA4
    #define PIN_GONG            PIN_PA3
    #define PIN_BUSY            PIN_PA5

    #define PIN_BTN_PREVIOUS    PIN_PB0
    #define PIN_BTN_NEXT        PIN_PB1
    #define PIN_BTN_MENU        PIN_PA6
    #define PIN_BTN_STOP        PIN_PA7

// Playmodes

    #define MODE_ONE    0  // Play a single ringtone
    #define MODE_NEXT   1  // Play ringtones in order "next file"
    #define MODE_RANDOM 2  // Play ringtones in shuffle

// Player Events

    #define PLAYER_NO_EVENT       0 // No event
    #define PLAYER_FILE_ERROR     1 // File not found or other file error
    #define PLAYER_BUSY_OFF       2 // End of playing (BUSY rising L->H now)
    #define PLAYER_TIMEOUT        3 // Timeout event - player not responded
    #define PLAYER_BUSY_ON        4 // Player switched to busy status
    #define PLAYER_OTHER_ERROR    5 // Other Players error
    #define PLAYER_CARD_INSERTED  6
    #define PLAYER_CARD_REMOVED   7

// Status

    #define STATUS_IDLE           0x00 // Idle; not playing
    #define STATUS_PLAY_ONE       0x01 // Playing a ringtone in MODE_ONE mode
    #define STATUS_PLAY_NEXT      0x02 // Playing a ringtone in MODE_NEXT mode
    #define STATUS_PLAY_RANDOM    0x03 // Playing a ringtone in MODE_RANDOM mode
    #define STATUS_PLAY           0x03 // mask for 01,02,03
    #define STATUS_NEXT_FILE      0x04 // Set and play the next ringtone
    #define STATUS_PREVIOUS_FILE  0x08 // Set and play the previous ringtone
    #define STATUS_MESSAGE        0x10 // Playing message
    #define STATUS_PLAY_DEFAULT   0x40 // Playing default ringtone
    #define STATUS_PLAY_TEST      0x80 // Playing ringtone via Test
    #define STATUS_CARD_REMOVED   0x20 // MicroSD card is removed

// Gong array index
    
    #define GONGS  2 //count of gongs
    
    #define EDITED 0
    #define NORMAL 1
   

// messages -  mp3 file numbers

    #define FOLDER_MESSAGE                  51  // Messages folder number
    
    #define MESSAGE_NOTHING                 0   // No messsage
    #define MESSAGE_FOLDER_1                1   // "Priečinok 1"
    #define MESSAGE_FOLDER_2                2   // "Priečinok 2"
    #define MESSAGE_FOLDER_3                3   // "Priečinok 3"
    #define MESSAGE_FOLDER_4                4   // "Priečinok 4"
    #define MESSAGE_FOLDER_5                5   // "Priečinok 5"
    #define MESSAGE_FOLDER_6                6   // "Priečinok 6"
    #define MESSAGE_FOLDER_7                7   // "Priečinok 7"
    #define MESSAGE_FOLDER_8                8   // "Priečinok 8"
    #define MESSAGE_FOLDER_9                9   // "Priečinok 9"
    #define MESSAGE_MODE_ONE                20  // "Prehrávanie jedného zvonenia"
    #define MESSAGE_MODE_NEXT               21  // "Postupné prehrávanie zvonení"
    #define MESSAGE_MODE_RANDOM             22  // "Prehrávanie v náhodnom poradí"
    #define MESSAGE_VOLUME_SETTING          30  // "Nastavenie hlasitosti"
    #define MESSAGE_FOLDER_EMPTY            40  // "Prázdny priečinok"
    #define MESSAGE_LAST_FILE_IN_FOLDER     41  // "Posledný súbor v priečinku"
    #define MESSAGE_FIRST_FILE_IN_FOLDER    42  // "Prvý súbor v priečinku"
    #define MESSAGE_FILE_SYSTEM_ERROR       43  // "Chyba v súborovom systéme"
    #define MESSAGE_CANCEL                  91  // Chyba, zrušenie (tri klesajúce tóny)
    #define MESSAGE_PREFERENCES_SAVED       92  // "Nastavenia boli uložené"
    #define MESSAGE_START                   93  // Úvodná zvučka - štart zvončeka
    #define MESSAGE_RESET_BELL              140 // "Bol vykonaný reset zvončeka"
    #define MESSAGE_DEFAULT_GONG            255 // "Predvolené zvonenie - použije sa ak požadované zvonenie nie je k dispozícii"

// Blink status

    #define BLINK_IDLE      0
    #define BLINK_EDIT      1
    #define BLINK_PLAY      2
    #define BLINK_MISSING_DEFAULT_GONG 3
    #define BLINK_GENERAL_ERROR 4


// Other

    #define DEFAULT_VOLUME 20   
    #define VOLUME_STEP     5
    #define VOLUME_MIN      10
    #define VOLUME_MAX      30
    #define RESET_TIME     10000 
    #define LOCK_BUTTONS_TIME 2000 
    #define EDIT_TIME 20000ul //20 sec



#ifdef DEBUG_ON
    #define NL Serial.println()
    #define T(x) Serial.print(x)
    #define D(x) Serial.print(x, DEC)
    #define H(x) Serial.print(x, HEX)

#else
    #define NL
    #define T(x)
    #define D(x)
    #define H(x)

#endif


// LedBlink pattern
  const unsigned int blink_start[] = {LED_BLINK_REPEAT_MODE | 5, 100, 100, 100, 100, 100, 500, 0};
  const unsigned int blink_player_error[] = {LED_BLINK_TIME_MODE | 2000, 20, 20, 0};
  const unsigned int blink_edit[] = {LED_BLINK_INFINITY_MODE, 1000, 50, 0};
  const unsigned int blink_idle[] = {LED_BLINK_INFINITY_MODE, 100, 5000, 0};
  const unsigned int blink_gong[] = {LED_BLINK_INFINITY_MODE, 200, 200, 0};
  const unsigned int blink_missing_default_gong[] = {LED_BLINK_INFINITY_MODE, 50, 50, 0};
  const unsigned int blink_general_error[] =  {LED_BLINK_INFINITY_MODE, 50, 50, 0};
  
// settings structure for Gong

struct GongSettings {
    uint8_t folder; //folder number 01-09
    uint8_t file;   //file 001-255
    uint8_t mode;   //playmode MODE_ONE | MODE_RANDOM | MODE_NEXT
    uint8_t volume; //volume 0-30
    bool first;     //first file in folder - "left margin"
    bool last;      //last file  in folder - "right margin"
    bool ready;     //file ready (doesn't generate new file number)  
};


// Timer

struct Timer {
  public:
    bool active;
    unsigned long timer;
    unsigned long time; //setted time

    // Initialize timer with timeout time
    void begin(unsigned long timeout) {
      active = false;
      timer = 0;
      time  = timeout;
    } 

    // Start timer now!
    void start() {
      active = true;
      timer = millis();
    }

    // Stop timer now
    void stop() {
      active = false;
    }

    //It returns True if timeout
    bool isTime() {
      if (!active) return false;
      if (millis() - timer > time) {
        active = false;
        return true;
      }
      else {
        return false;
      }
    }
};




// Prototypes of Action functions. These are the functions that are called on the button event

  void playAction(bool test = false); //test=true - test mode
  void stopAction();
  void testAction();
  void prevFileAction();
  void nextFileAction();
  void prevFolderAction();
  void nextFolderAction();
  void volumeUpAction();
  void volumeDownAction();
  void nextPlayModeAction();
  void saveAction();

// Prototypes other function

  void stop(); //stop playing
  void play(uint8_t folder , uint8_t file); //play file/folder
  void prevFile(); // set previous file in current folder
  void nextFile(); // set next file in current folder
  void prevFolder(); // set previous folder and file set to 1
  void nextFolder(); // set next folder and file set to 1
  void volumeUp(); //volume up
  void volumeDown(); //volume down
  void tryReset(); // wait 10sec and then reset
  void init_gong(); // initialize machine
  void unblockLockedButtons();
  void blink(int status);
  bool load(); // loads settings from EEPROM
  void save(); // saves settings to EEPROM
  
// DFPlayer functions
  bool getBusy(); //Returns false if BUSY is HIGH, return true if BUSY is LOW
  int  edgeBusy(bool reset = true); // returns BUSY edge: to LOW 1, to HIGH -1, no change 0
  void playerUpdate(); // updates player; call regularly
  int  playerEvent();  // gets player event
  int  getFileCountsInFolder(uint8_t folder); // get file counts in folder

// Debug print functions
  void printEvent(int event);
  void printStatus(int status);

  
// Button instances
  RealButton btnPrev(PIN_BTN_PREVIOUS);
  RealButton btnNext(PIN_BTN_NEXT);
  RealButton btnMode(PIN_BTN_MENU);
  RealButton btnStop(PIN_BTN_STOP);
  RealButton btnGong(PIN_GONG);
  
  // DFRPlayer instance
  DFRobotDFPlayerMini myDFPlayer;

  // Led blink instance
  LedBlink led;

  int blink_status = BLINK_IDLE;

  // Status of machine
  uint8_t status = STATUS_IDLE;  

  // Traffic light - waiting for player response (blocking buttons)
  bool wait_for_player_response = false; 

  struct GongSettings gong[GONGS]; //saved settings

  int gong_index = EDITED; // NORMAL | EDITE

  //timer for edited settings
  struct Timer editTimer;

  // flag EDIT mode
  bool edit_flag = false;

////////////////////////////////// MAIN //////////////////////////////////////

/// @brief Main SETUP function
void setup() {
  
  Serial1.begin(9600);  //DFPlayer
  Serial.begin(115200); //Serial debug
  led.begin(PIN_LED, OFF);
 
  NL; T("Start TinTinnabulum A.00.03"); NL; NL;
  
  // Buttons starting 
  btnPrev.start();
  btnNext.start();
  btnMode.start();
  btnStop.start();
  btnGong.start();
  
  editTimer.begin(EDIT_TIME);
 
  // Initialize player
  while ( !myDFPlayer.begin(Serial1, true, true)) {
   led.blink(blink_player_error);
   while(led.isBlinking) led.update();
  };
  
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(25);
  
  init_gong(); // initialize control variables
  if (!load()) {
    delay(100);
    while(getBusy());
  }
  edit_flag = false;
  
  myDFPlayer.playFolder(FOLDER_MESSAGE, MESSAGE_START);
  led.blink(blink_start);
}

/// @brief Main LOOP function
void loop() {
  
   while (status == STATUS_CARD_REMOVED) {
        led.update();
        playerUpdate();
      }

  if (btnStop.pressed() && btnMode.pressed()) 
    tryReset();
   
  if (btnGong.onPress()) {
      NL;
      playAction();
  } 

  if (!wait_for_player_response) {
    if (btnPrev.pressed() || btnNext.pressed() || btnMode.pressed()) {
      editTimer.start();  // restart timer if btn 1|2|3 is pressed
      if (!edit_flag) {
        edit_flag = true;
        
        gong[EDITED] = gong[NORMAL]; // copy settings
        gong_index = EDITED;

        blink(BLINK_EDIT);
      }
    } 
    else {
        if (editTimer.isTime() || btnStop.onClick(false)) { // if btnStop.onClick or Timer ends
          gong[EDITED] = gong[NORMAL]; // cancel edited settings: set to normal
          gong_index = NORMAL;
          edit_flag = false;

          editTimer.stop();
          led.stop();
        } 
    }

    
    if(btnMode.pressed()) {
      if(btnPrev.onClick()) {
        NL;
        btnMode.reset();
        volumeDownAction();
      } 

      else if(btnNext.onClick()) {
        NL;
        btnMode.reset();
        volumeUpAction();
      }

      else if (btnMode.onLong()) {
        NL;
        testAction();
      }
    } 
    
    

    if (btnPrev.onClick()) {
      NL;
      prevFileAction();
    }
    
    else if (btnNext.onClick()) {
      NL;
      nextFileAction();
    } 

    else if (btnMode.onClick()) {
      NL;
      nextPlayModeAction();
    } 

    else if (btnPrev.onLong()) {
      NL;
      prevFolderAction();
    } 

    else if (btnNext.onLong()) {
      NL;
      nextFolderAction();  
    } 

    
    else if (btnStop.onClick()) {
      NL;
      stopAction();
    } 

    else if (btnStop.onLong()) {
      NL;
      led.stop(OFF);
      saveAction();
    }   
  }
    // unblock buttons if status is idle min. 2 sec.
      unblockLockedButtons();

    // Update DFR0299 Player
      playerUpdate();
  
    // Update buttons
      btnPrev.update();
      btnNext.update();
      btnMode.update();
      btnStop.update();
      btnGong.update();
 
    // Update LED blinking
      led.update();

  if (status == STATUS_IDLE) {
    if (edit_flag) {
      blink(BLINK_EDIT);
    }
    else {
      blink(BLINK_IDLE);
    }
  }

  else if ((status & STATUS_PLAY) && !(status & STATUS_PLAY_TEST) ) {
    blink(BLINK_PLAY);
  }

}

//////////////////////////// AUXILIARY FUNCTIONS /////////////////////////////

/// @brief It starts flashing according to the command in the parameter
/// @param status One of the constants BLINK_*
void blink(int status) {
  static int current_status = BLINK_IDLE;
  
  if (status != current_status) {
    current_status = status;
    switch (status) {
      case BLINK_IDLE:
        led.blink(blink_idle);
        break;
      
      case BLINK_EDIT:
        led.blink(blink_edit);
        break;
      
      case BLINK_PLAY:
        led.blink(blink_gong);
        break;
      
      case BLINK_MISSING_DEFAULT_GONG:
        led.blink(blink_missing_default_gong);
        break;

      case BLINK_GENERAL_ERROR:
        led.blink(blink_general_error);
        break;
    }
  }
}

/// @brief The function returns the moment of switching the BUSY state
/// @param reset 
/// @return BUSY state: -1 - to OFF, 1 - to ON, 0 - the BUSY state has not changed
int edgeBusy(bool reset = true) {
  static bool previous_busy = false;
  bool busy = digitalRead(PIN_BUSY) == LOW;
  
  if (busy != previous_busy) {
    if (reset) 
      previous_busy = busy;
    return busy ? 1 : -1;
  }
  else 
    return 0;
}

/// @brief It returns state of BUSY pin. 
/// @return true - LOW (ON), false - HIGH (OFF)
bool getBusy() {
  return digitalRead(PIN_BUSY) == LOW;
}

/// @brief Counts the files in a folder
/// @param folder Folder number
/// @return Number of files
int getFileCountsInFolder(uint8_t folder) {
      myDFPlayer.readFileCountsInFolder(folder); //deBUG zzz need read 2x!
      return myDFPlayer.readFileCountsInFolder(folder);
}

/// @brief Initializing settings
void init_gong() {
  // loading settings from eeprom
 
  // index 0 = EDITED
  gong[EDITED].file = 1;
  gong[EDITED].folder = 1;
  gong[EDITED].first = true;
  gong[EDITED].last = false;
  gong[EDITED].ready = true;
  gong[EDITED].mode = MODE_ONE;
  gong[EDITED].volume = DEFAULT_VOLUME;
  
  gong[NORMAL] = gong[EDITED];
  
  gong_index = EDITED;
  
  edit_flag = false;
  wait_for_player_response = false;
  status = STATUS_IDLE;

}

/// @brief Start playing the file
/// @param folder Folder number (1-9)
/// @param file File number (1 -255)
void play(uint8_t folder, uint8_t file) {
  
  T("> PLAY");
  T(", folder = ");
  D(folder);
  T(", file = ");
  D(file);
  NL;

  stop();
  T("PLAYER> busy switch to ");
  int b;
  b = edgeBusy();
  D(b); NL;

  T("> DFR playFolder()! ");
  wait_for_player_response = true;
  myDFPlayer.volume(gong[gong_index].volume);
  delay(10);
  myDFPlayer.playFolder(folder, file);
  T("Done."); NL;
}

/// @brief Stop playback and cancel editing mode
void stop() {
  unsigned int timer = millis();
  
  myDFPlayer.stop();
  delay(50);
  
  timer = millis();

  while(getBusy());

  timer = (unsigned int)millis() - timer; 
  T("> (stop) - delay time: ");
  D(timer);
  T(" ms");
  NL;
}

/// @brief The function checks for pressing the O and M buttons for 10 seconds, then performs a reset
void tryReset() {
  // wait 10 sec
  unsigned int timer = millis();
  bool doReset = true;

  while ( ((unsigned int) millis()) - timer  < RESET_TIME) {
    if (!btnStop.pressed() || !btnMode.pressed()) {
      doReset = false;
      break;
    }
    
    btnStop.update();
    btnMode.update();
  }

  if (doReset) {
    setup();
    int event = playerEvent();
    while (event == PLAYER_NO_EVENT || event == PLAYER_BUSY_OFF) {
      event = playerEvent();
      led.update();
    }
    while(getBusy()) { //wait for end play jingel
      led.update();
    }
  }
}

/// @brief The function unlocks the buttons after 2 seconds of player inactivity
void unblockLockedButtons() {
  static unsigned int lock_timer = millis();
  static bool previous_locked = false;

    if (previous_locked != wait_for_player_response) {
      previous_locked = wait_for_player_response;
      if (wait_for_player_response) {
        lock_timer = millis();
      }
    }
  
  if (wait_for_player_response) {
    if (((unsigned int) millis()) - lock_timer > LOCK_BUTTONS_TIME) {
      stopAction(); //unlock
    }
  }
}

/// @brief The function returns the status and message from the player
/// @return Player event number PLAYER_*
int playerEvent() {
  int busy = edgeBusy();
  int event = PLAYER_NO_EVENT;
  
  // test Busy event
  if (busy != 0) {
    event = (busy == 1) ? PLAYER_BUSY_ON : PLAYER_BUSY_OFF;
  }

  // test DFR events
  else if(myDFPlayer.available()) {

    uint8_t type = myDFPlayer.readType();
    int value = myDFPlayer.read();
    
    switch (type) {
      case DFPlayerError: 
        switch (value) {
          case FileMismatch:
          case FileIndexOut:
            event = PLAYER_FILE_ERROR;
            break;
        }
        break;

      case DFPlayerCardInserted:
        event = PLAYER_CARD_INSERTED;
        break;

      case DFPlayerCardRemoved:
        event = PLAYER_CARD_REMOVED;
        break;
            
      default:
        event = PLAYER_OTHER_ERROR;
        break;
    }
  }

  return event;
}

////////////////////////////////// DEBUG PRINTS ///////////////////////////////////

/// @brief Writes the name of the event constant to Serial
/// @param event Event constant PLAYER_*
void printEvent(int event) {

  if (event == PLAYER_NO_EVENT) return;
  T("> Player event: ");
  switch (event) {
    case PLAYER_FILE_ERROR:
      T("PLAYER_FILE_ERROR");
      break;
    case PLAYER_BUSY_OFF:
      T("PLAYER_BUSY_OFF");
      break;
    case PLAYER_TIMEOUT:
      T("PLAYER_TIMEOUT");
      break;
    case PLAYER_BUSY_ON:
      T("PLAYER_BUSY_ON");
      break;
    case PLAYER_OTHER_ERROR:
      T("PLAYER_OTHER_ERROR");
      break;
    default:
      T("UNKNOWN EVENT ("); D(event);T("("); 
      break;
  }
  NL;
}

/// @brief Writes the name of the status constant to Serial
/// @param status Status constant STATUS_*
void printStatus(int status) {
  switch (status) {
    case STATUS_IDLE: T("STATUS_IDLE"); break;
    case STATUS_PLAY_ONE: T("STATUS_PLAY_ONE"); break;
    case STATUS_PLAY_NEXT: T("STATUS_PLAY_NEXT"); break;
    case STATUS_PLAY_RANDOM: T("STATUS_PLAY_RANDOM"); break;
    case STATUS_NEXT_FILE: T("STATUS_NEXT_FILE"); break;
    case STATUS_PREVIOUS_FILE: T("STATUS_PREVIOUS_FILE"); break;
    case STATUS_MESSAGE: T("STATUS_MESSAGE"); break;
    case STATUS_PLAY_DEFAULT: T("STATUS_PLAY_DEFAULT"); break;
    case STATUS_PLAY_TEST: T("STATUS_PLAY_TEST"); break;
    default: T("Unknown STATUS");
  } 
}

///////////////////////////////// ACTION FUNCTIONS ////////////////////////////////

void playAction(bool test = false) {
  
  // Debug
  if (test) {
     NL; T("* Action: Test Action"); NL;
  } 
  else{
     NL; T("* Action: Play Action"); NL;
  }
 
  // If gong is playing then skip function
  if ((status & STATUS_PLAY) && !(status & STATUS_PLAY_TEST)) {
    T("> Skip, gong is playing now!"); NL;
    return;
  }
  
  blink(BLINK_PLAY);

  gong_index = NORMAL;

  // If file is not ready to play - generate new file number
  if (!gong[NORMAL].ready) {
    if (gong[NORMAL].mode == MODE_NEXT) {
      gong[NORMAL].file++;
    } 

    else if  (gong[NORMAL].mode == MODE_RANDOM) {
      int file_count = getFileCountsInFolder(gong[NORMAL].folder);
      T(">  myDFPlayer.readFileCountsInFolder("); D(file_count); T(") = ");
      gong[NORMAL].file = random(1, file_count + 1);
      D(gong[NORMAL].file); NL;
    }
  }

  
  // set properly status
  switch (gong[NORMAL].mode) {
    case MODE_ONE:
      status = STATUS_PLAY_ONE;
      break;
    
    case MODE_NEXT:
      status = STATUS_PLAY_NEXT;
      break;

    case MODE_RANDOM:
      status = STATUS_PLAY_RANDOM;
      break;
  }

  if (test) {
    status |= STATUS_PLAY_TEST;
    gong[NORMAL].ready = true;
  }
  else {
    gong[NORMAL].ready = false;
  }
  
  play(gong[NORMAL].folder, gong[NORMAL].file);
}

void stopAction() {
  NL; T("* Action: Stop"); NL;

  status = STATUS_IDLE;
  
  stop();
  wait_for_player_response = false;
  gong[EDITED] = gong[NORMAL];
  gong_index = NORMAL;
}

void testAction() {
  playAction(true);
}

void prevFileAction() {
  NL; T("* Action: Previous File Action"); NL;
  //play(FOLDER_BUTTONS, MESSAGE_BUTTON_PREVIOUS);
  prevFile();
}

void nextFileAction() {
  NL; T("* Action: Next File Action"); NL;
  nextFile();
}

void prevFolderAction() {
  //T("* Action: Previous Folder Action"); NL;
  //play(FOLDER_BUTTONS, MESSAGE_BUTTON_PREVIOUS_FOLDER);
  NL; T("* Action: Previous Folder Action"); NL;
  prevFolder();
}

void nextFolderAction() {
  //T("* Action: Next Folder Action"); NL;
  //play(FOLDER_BUTTONS, MESSAGE_BUTTON_NEXT_FOLDER);
  NL; T("* Action: Next Folder Action"); NL;
  nextFolder();
}

void volumeUpAction() {
  NL; T("* Action: Volume Up"); NL;
  volumeUp();
}

void volumeDownAction() {
  NL; T("* Action: Volume Down"); NL;
  volumeDown();
}

void nextPlayModeAction() {
  NL; T("* Action: Next Play Mode Action"); NL;
  //play(FOLDER_BUTTONS, MESSAGE_BUTTON_MENU);
  
  //Cyclic switch mode (there are 3 modes 0,1,2)
  gong[EDITED].mode ++;
  gong[EDITED].mode %= 3;
  
  status = STATUS_MESSAGE;

  T("> Mode set to ");
  switch (gong[EDITED].mode) {
    case MODE_ONE:
       T("ONE"); NL;
      play(FOLDER_MESSAGE, MESSAGE_MODE_ONE);
      break;
    
    case MODE_NEXT:
      T("NEXT"); NL;
      play(FOLDER_MESSAGE, MESSAGE_MODE_NEXT);
      break;

    case MODE_RANDOM:
      T("RANDOM"); NL;
      play(FOLDER_MESSAGE, MESSAGE_MODE_RANDOM);
      break;
  }
}

void saveAction() {
  NL; T("* Action: Save Action"); NL;
    status = STATUS_MESSAGE;
    play(FOLDER_MESSAGE, MESSAGE_PREFERENCES_SAVED);
    gong[NORMAL] = gong[EDITED];
    gong_index = NORMAL;
    edit_flag = false;
    save();
}

/////////////////////////////// EXECUTIVE FUNCTIONS //////////////////////////////////

/// @brief Tests whether the player sent the event and handles it
void playerUpdate() {
  int player_event = playerEvent();
  if (player_event == PLAYER_NO_EVENT) return;
    
    switch (player_event) {
      
      case PLAYER_FILE_ERROR:
        wait_for_player_response = false;
        T("Update PLAYER> File Error, status: "); 
        printStatus(status);
        NL;

        switch (status & ~STATUS_PLAY_TEST) {
          case STATUS_PLAY_ONE:
            play(FOLDER_MESSAGE, MESSAGE_DEFAULT_GONG);
            status = STATUS_PLAY_DEFAULT;
            break;
          
          case STATUS_PLAY_NEXT:
          case STATUS_PLAY_RANDOM:
            if ( gong[gong_index].file == 1) {
              play(FOLDER_MESSAGE, MESSAGE_DEFAULT_GONG);
              status = STATUS_PLAY_DEFAULT;
              blink(BLINK_MISSING_DEFAULT_GONG);
            }
            else {

              gong[gong_index].file = 1;
              gong[gong_index].first = false;
              gong[gong_index].last = false;
              gong[gong_index].ready = bool(status & STATUS_PLAY_TEST);
              play(gong[gong_index].folder, gong[gong_index].file);
            }

            break;
          
          case STATUS_NEXT_FILE:
              if ((getFileCountsInFolder(gong[gong_index].folder)  < 1) || 
                  (gong[gong_index].file == 1) 
                 ) {
                      status = STATUS_MESSAGE;

                      gong[gong_index].first = true;
                      gong[gong_index].file = 1;
                      
                      play(FOLDER_MESSAGE, MESSAGE_FOLDER_EMPTY);
                    } 
              
              else {

                gong[gong_index].last  = true;
                gong[gong_index].first = false;
                gong[gong_index].ready = true;
                gong[gong_index].file--;
                
                status = STATUS_MESSAGE;
                play(FOLDER_MESSAGE, MESSAGE_LAST_FILE_IN_FOLDER);                

                
              }
              break;

          case STATUS_PREVIOUS_FILE:
            if ((gong[gong_index].file == 1 )
                  || (getFileCountsInFolder(gong[gong_index].folder) < 1)) {
                    status = STATUS_MESSAGE;
                    gong[gong_index].first = true;
                    gong[gong_index].file = 1;
                    play(FOLDER_MESSAGE, MESSAGE_FOLDER_EMPTY);
                  }
            else {
              status = STATUS_MESSAGE;
              play(FOLDER_MESSAGE, MESSAGE_FILE_SYSTEM_ERROR);
            }
            break;


            case STATUS_PLAY_DEFAULT:
              
              status = STATUS_PLAY_DEFAULT;
              blink(BLINK_MISSING_DEFAULT_GONG);

            break;
          //other cases must by programmed (next, prev...)

        }
        break;

      case PLAYER_BUSY_OFF:
        if (status != STATUS_CARD_REMOVED) {
          T("Update PLAYER> Player Busy OFF"); NL; 
          status = STATUS_IDLE;
        }
        break;

      case PLAYER_BUSY_ON:
        wait_for_player_response = false;
        T("Update PLAYER> Player Busy ON "); NL; 
        break;

      case PLAYER_CARD_REMOVED:

        T("Update PLAYER> CARD REMOVED"); NL;
        blink(BLINK_GENERAL_ERROR);
        status = STATUS_CARD_REMOVED;
        break;

      case PLAYER_CARD_INSERTED:
         T("> Update PLAYER> CARD INSERTED"); NL;
        setup();
        init_gong();
        break;

      default:
         wait_for_player_response = false;
        break;
   }
}

/// @brief Sets the next file in the current folder
void nextFile() {
  const int local_gong_index = EDITED;
  if (gong[local_gong_index].first || gong[local_gong_index].last) {
    gong[local_gong_index].file = 1;
  }
  else {
    gong[local_gong_index].file ++;
  }

  gong[local_gong_index].first = false;
  gong[local_gong_index].last = false;
  gong[local_gong_index].ready = true;

  status = STATUS_NEXT_FILE;
  
  play(gong[local_gong_index].folder, gong[local_gong_index].file);
}

/// @brief Sets the previous file in the current folder
void prevFile() {
  const int local_gong_index = EDITED;  
  if (gong[local_gong_index].file == 1) {
    gong[local_gong_index].first = true;

    status = STATUS_MESSAGE;

    play(FOLDER_MESSAGE, MESSAGE_FIRST_FILE_IN_FOLDER);
  }

  else {
    if (!gong[local_gong_index].last) 
      gong[local_gong_index].file --;
    
    gong[local_gong_index].last = false;
    gong[local_gong_index].ready = true;

    status = STATUS_PREVIOUS_FILE; 

    play(gong[local_gong_index].folder, gong[local_gong_index].file);
  }
}

/// @brief Sets the next folder in order
void prevFolder() { 
  const int local_gong_index = EDITED; 

  if (gong[local_gong_index].folder <= 1) {
    
    gong[local_gong_index].folder = 1;
    
  } 
  else if (gong[local_gong_index].folder > 9) {
    
    gong[local_gong_index].folder = 9;

  } 
  else {
    gong[local_gong_index].folder --;
  }

  status = STATUS_MESSAGE;

  gong[local_gong_index].file = 1;
  gong[local_gong_index].last = false;
  gong[local_gong_index].first = true;
  gong[local_gong_index].ready = true;

  play(FOLDER_MESSAGE, gong[local_gong_index].folder - 1 + MESSAGE_FOLDER_1);
}

/// @brief Sets the previous folder in order
void nextFolder() { 
  const int local_gong_index = EDITED; 

  if (gong[local_gong_index].folder < 1) {
    
    gong[local_gong_index].folder = 1;
    
  } 
  else if (gong[local_gong_index].folder >= 9) {
    
    gong[local_gong_index].folder = 9;

  } 
  else {
    gong[local_gong_index].folder ++;
  }

  status = STATUS_MESSAGE;

  gong[local_gong_index].file = 1;
  gong[local_gong_index].last = false;
  gong[local_gong_index].first = true;
  gong[local_gong_index].ready = true;

  play(FOLDER_MESSAGE, gong[local_gong_index].folder - 1 + MESSAGE_FOLDER_1);

}

/// @brief Sets volume up 
void volumeUp() {
  const int local_gong_index = EDITED;
  int volume =  gong[local_gong_index].volume;
  
  stop();
  
  // Volume Up
  volume += VOLUME_STEP;

  // Correct
  if (volume > VOLUME_MAX - VOLUME_STEP + 1) volume = VOLUME_MAX;
  
  // Set
  myDFPlayer.volume(volume);
  gong[local_gong_index].volume = volume;

  // Play info

  status = STATUS_MESSAGE;
  play(FOLDER_MESSAGE, MESSAGE_VOLUME_SETTING);

}

/// @brief Sets volume down
void volumeDown(){
  const int local_gong_index = EDITED;
  int volume =  gong[local_gong_index].volume;
  
  stop();

  // Volume Up
  volume -= VOLUME_STEP;

  // Correct
  if (volume < VOLUME_MIN + VOLUME_STEP - 1) volume = VOLUME_MIN;
  
  // Set
  myDFPlayer.volume(volume);
  gong[local_gong_index].volume = volume;

  // Play info
  status = STATUS_MESSAGE;
  play(FOLDER_MESSAGE, MESSAGE_VOLUME_SETTING);
  
}

/// @brief Saves settings to EEPROM
void save() {
  struct GongSettings data;
  int adr = 0;
  
  T("> (save) The settings have been saved"); NL;

  data = gong[NORMAL];
  data.last  = false;
  data.first = false;
  data.ready = false;

  EEPROM.put(adr, data); //data
  
  //verify 
  data.file = ~data.file;
  data.folder = ~data.folder;
  data.mode = ~data.mode;
  data.volume = ~data.volume;
  

  adr += sizeof(data); 
  EEPROM.put(adr, data); // verify 
}

/// @brief Loads setting form EEPROM
/// @return success - true
bool load() {
  bool return_value = false;
  T("> (load) Load settings from EEPROM - ");
  struct GongSettings data, data2;
  int adr = 0;
  EEPROM.get(adr, data);
  adr += sizeof(data);
  EEPROM.get(adr, data2);
  
  if ((data.file + data2.file == 0xFF) && (data.folder + data2.folder == 0xFF) && (data.mode + data2.mode == 0xFF) &&(data.volume + data2.volume == 0xFF)) {
    gong[NORMAL] = data;  
    gong[NORMAL].first = false;
    gong[NORMAL].last  = false;
    gong[NORMAL].ready = true;

    return_value = true; 
    T("OK"); NL; 
  }
  else {
    T("ERROR"); NL;
    T("!!! RESET DOORBELL !!!"); NL;

    init_gong();
    save();
    status = STATUS_MESSAGE;
    play(FOLDER_MESSAGE, MESSAGE_RESET_BELL);
    delay(100);
  }

  return return_value;
}
