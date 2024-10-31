# TinTinNabulum A
**The melodious doorbell s with DFPlayer Mini - DFR0299 and ATTiny1624 microcontroller**

![obrázok](https://github.com/user-attachments/assets/878f3557-7c21-4197-90a8-3d792b67989c)



> TinTinNabulum A is a home melodic doorbell that rings by playing a melody.
> Ringtones are stored on a microSD card and can be easily changed or supplemented.

## Features

* Ability to store more than 2000 ringtones
* Ability to divide ringtones into 9 separate folders
* Possibility to start ringing with a connected button or a voltage signal from another source
* Sound power 3 Watt
* DC power supply 9V (max. 1 A)

## Doorbell control

The bell is set with four buttons, you can set which melody will be used for ringing.
The buttons are:
> * "**<**" - Previous file / Previous folder
> * "**>**" - Next file / Next folder
> * "**M**" - Playmode / Test play
> * "**O**" - Stop, Cancel / Save settings


Each button has two functions:
1. First function - when the button is briefly pressed and released
2. Second function - when you press and hold the button for a long time (2 seconds)

Some functions are available by pressing two buttons at the same time:

> * "**M**" + "**<**" - Volume down
> * "**M**" + "**>**" - Volume up
> * "**M**" + "**O**" - Reset the doorbell and set the default settings

## Ringing by button or voltage signal

Ringing can be triggered in two ways:
* by shorting the control wires - with an external button
* by applying a voltage signal to the control wires

According to the chosen method, you must to set the DIP-switches on the circuit board of the doorbell.

You also set the size of the AC or DC control voltage with the switch:
* 4 V - 10 V
* 7 V - 20 V

Observe correct polarity when using DC voltage

The required current flowing through the control wires is 3 - 10 mA.

## Ringtones

Ringtones must be in an MP3 audio file. (Sampling rates (kHz): 8/11.025/12/16/22.05/24/32/44.1/48).

The files are stored in folders on a micro SD card, which can have a maximum capacity of 32 GB and supports FAT16, FAT32 file system. The card is inserted in the slot of the player.

You can upload files with ringtones to folders 01 to 09. 

Ringtone file names must be in the format 001.mp3, 002.mp3, etc. up to 255.mp3. File names must form an uninterrupted sequence starting with 001.mp3

There must also be folder 51 on the microSD card with mp3 sound files of notifications. Notifications are played while setting the doorbell with the buttons.

## Software
The doorbell software was created in the **PlatformIO** using Arduino framework for the **ATTiny1624** microcontroller.
It consists of the main file main.cpp and three external libraries:

* Button - the Button library provides button operation including software debouncing
* Led Blink - LED Blink library provides asynchronous control of LED flashing according to control commands
* DFRobotDFPlayerMini - Software control of the DFR0299 player via a serial interface
  
> The program can also be compiled and uploaded in the **Arduino IDE 2**.
> The **MegaTinyCore** package by Spence Konde must be installed in the Arduino IDE first. It must be entered in the field in the Preferences: "Additional boards manager URLs address" *http://drazzy.com/package_drazzy.com_index.json*

The main sketch is in the Arduino folder and the libraries are in the lib directory - these must be uploaded to the Arduino libraries directory

### Uploading the program to the microcontroller

The Attiny 1624 microcontroller is programmed via the UPDI interface. There are pin headers on the printed circuit board where the programmer is connected. PCB must be powered during programming.

![obrázok](https://github.com/user-attachments/assets/64cff65f-db5d-4a3a-a12f-fa6ba589f899)


>You can make a suitable jtag2updi programmer from an Arduino Nano. Instructions can be found at:
> https://daumemo.com/diy-updi-usb-programmer-which-can-be-made-with-cheap-hardware/

![obrázok](https://github.com/user-attachments/assets/83673158-ce7a-4c45-9154-d9831187b445)


## Photos

> Home made PCBs
> Top - Top panel buttons PcB
> Bottom - Main PCB

![obrázok](https://github.com/user-attachments/assets/bc028b53-51cd-477b-b0b5-5a6ba27f9e7a)


> Assembled PCB

![obrázok](https://github.com/user-attachments/assets/8f680a6e-c2ca-4be7-8d0d-55b581fdfd7d)


> Rear panel with connectors for button or voltage signal and power supply

![obrázok](https://github.com/user-attachments/assets/070a5b5c-7ca1-4f6b-9531-dc49c3dd3f07)



> Buttons - top panel

![obrázok](https://github.com/user-attachments/assets/64fe1e94-9d25-4d64-a116-491a40f98bfc)

> PCB in the speaker box Genius SP-HF280 

![obrázok](https://github.com/user-attachments/assets/060cb161-d37e-4030-8420-b0b94abe4c10)




















