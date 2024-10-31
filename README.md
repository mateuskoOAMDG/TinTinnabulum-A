# TinTinNabulum A
**The melodious doorbell s with DFPlayer Mini - DFR0299 and ATTiny1624 microcontroller**
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
The required current flowing through the control wires is 3 - 10 mA.

## Ringtones

Ringtones must be in an MP3 audio file. (Sampling rates (kHz): 8/11.025/12/16/22.05/24/32/44.1/48).

The files are stored in folders on a micro SD card, which can have a maximum capacity of 32 GB and supports FAT16, FAT32 file system. The card is inserted in the slot of the player.

You can upload files with ringtones to folders 01 to 09. 

Ringtone file names must be in the format 001.mp3, 002.mp3, etc. up to 255.mp3. File names must form an uninterrupted sequence starting with 001.mp3

There must also be folder 51 on the microSD card with mp3 sound files of announcements. Notifications are played while setting the doorbell with the buttons.




















