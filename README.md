# NERC Timer #

This is the codebase for the new NERC timer. The clock hub runs off a Mega 2560 with a pinout hat. This helps avoid interrupts when polling the timer.

## Equipment Pre-reqs ##

- USB Serial Receiver for machine used to pull timer, and other outputs from the arduino.
- Python, pyserial on whichever OS you are running the clock script from.
- Heavy gauge DMX cable for clock DMX line. Will pull 5A at 5VDC. Needs to be short run.
- Conceptinetics library for Arduino: https://sourceforge.net/projects/dmxlibraryforar/

### MicroSD Card Requirements ###

Up to 32GB drive, FAT16/FAT32. **MUST** be MBR format.

## Programming the Mega 2560 ##

You must remove the hat before programming. The hat is drawing enough current on the bus, it will not connect otherwise.

## Python Items ##

### Installing Python on Windows ###

Use the windows installer to install python: https://www.python.org/downloads/windows/

#### pyserial ####

Download files from: http://pypi.python.org/pypi/pyserial
Docs are at: https://pyserial.readthedocs.io/en/latest/pyserial.html#installation

Unpack the tar.gz. Nagivate in to the directories and locate setup.py. From command prompt or Powershell, run:

py setup.py install

### Installing Python/pyserial on Linux ###

apt install python3 python3-serial

## Parameters ##

### Sound Files ###

Files must be 128k. 320k CBR does not work.

Folder Structure
- Root
    - 01
        - 001xxxxx.mp3
        - 002xxxxx.mp3
        - etc..

| Serial Address | Description | Audio file in folder |
|----------------|-------------|----------------------|
| 0X1901 | Start (beep beep beep, boop) | 001xxxxx.mp3
| 0X1902 | Blue Ready | 002xxxxx.mp3
| 0X1903 | Red Ready | 003xxxxx.mp3
| 0X1904 | Tap Out | 004xxxxx.mp3
| 0X1905 | Stop | 005xxxxx.mp3

### Link to OBS ###

There is a serial output in the script that pipes to the wireless USB serial transmitter. pyserial script with serial receiver converts the output to a webpage for consumption in OBS.

### Timer Breakdown ###

Clock starts in standby. Time is set via command box. There are 2, 3, and 5 minute buttons, with indicator lamps.
In addition, there is a start, pause, and stop button. Pause button currently pauses clock, and needs to be pressed again to continue clock countdown.  
In standby both players need to press their color button to indicate that they are ready. Sounds indicate that they are ready.
StartTree kicks in, produces the start tones, then starts clock.
Either Red or Blue button can be pressed for tap out during fight, which also includes pause currently.

## Current items to change ##

- Adding a starttree countdown tone to the restart after pause
- Bonus wish: adding button tap to say you are ready upon pause
- DMX light time on starttree is very quick blips. Needs some incremental workup to intensity level perhaps?
- Start tree should stay green when match has started. Yellow/Amber when paused, and Red when stopped.
- Timer clock should turn yellow or amber on pause, with possible pulse in intensity.
 - Needs to be defined as a new color.
- Tap out lighting should be red flashes, then have the color of the square who tapped stay indicated for 10-20 seconds, then clear.
- Serial output to OBS should also include an indicator for player ready for both squares, then stop when fight starts.
- Serial output to OBS should also include an indicator for player who taps out. Tap Out/Winner as highlight over square.