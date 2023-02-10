# NERC Timer #

This is the codebase for the new NERC timer. The clock hub runs off a Mega 2560 with a DMX hat. This helps avoid interrupts when polling the timer.

## Equipment Pre-reqs ##

- Arduino Mega 2560, or similar. Needs the larger pin bank to accomodate the DMX Shield, and also performs better with the clock.
- LED Strip Lights to make your own Adafruit NeoPixel clock: https://learn.adafruit.com/adafruit-neopixel-uberguide
  - Do note, you will need heavy gauge (we used 14ga) wiring for clock line. Will pull 5A at 5VDC. Needs to be a shorter run as well to keep the power full.
- USB Serial Receiver for machine used to pull timer, and other outputs from the arduino.
  - Something like this: https://diydrones.com/profiles/blogs/announcing-the-3dr-radio-telemetry-system
- Python, and pyserial package on whichever OS you are running the clock script from.
- Conceptinetics library for Arduino: https://sourceforge.net/projects/dmxlibraryforar/
  - Other native arduino libs that are needed:
    - JC_Button
    - Adafruit NeoPixel
- Serial MP3 Player Card. Ours has a YX5300 chipset.
  - Something like this: https://www.amazon.com/HiLetgo-YX5300-Control-Serial-Arduino/dp/B0725RHR4D
- DMX Shield for Arduino. Ours uses a CTC-DRA-10-R2.
  - Docs found here: https://www.halloweenfreak.de/arduino/pdfs/DMX_Shield_Arduino_CTC-DRA-10-R2.pdf
- DMX Lights! Any type will do, it will all depend on what you want to accomplish with them. For our use cases, 3-9 channels were enough. We just needed to program RGB(AW), and have a healthy strobe effect.

### MicroSD Card Requirements ###

Up to 32GB drive, FAT16/FAT32. **MUST** be MBR format.

## Programming the Mega 2560 ##

You must switch the jumper on the DMX Shield from EN to (not)_EN_ The hat is draws enough current on the bus that it will not connect otherwise since the port will be saturated, and the built in self fuse will trip.

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

Supports MP3 and WAV files up to 48khz sampling. I am using 192kbps bitrate MP3s.

Folder Structure
- Root
    - 01
        - 001xxxxx.mp3
        - 002xxxxx.mp3
        - etc..

| Serial Address | Description | Audio file in folder |
|----------------|-------------|----------------------|
| 0X0101 | Start (beep beep beep, boop) | 001xxxxx.mp3 |
| 0X0102 | Blue Ready | 002xxxxx.mp3 |
| 0X0103 | Red Ready | 003xxxxx.mp3 |
| 0X0104 | Red Tap Out | 004xxxxx.mp3 |
| 0X0105 | Stop | 005xxxxx.mp3 |
| 0x0106 | Blue Tap Out | 006xxxxx.mp3 |
| 0x0107 | 2 Min Warning | 007xxxxx.mp3 |
| 0x0108 | 1 Min Warning | 008xxxxx.mp3 |
| 0x0109 | 10 sec Countdown Beep | 009xxxxx.mp3 |
| 0x010A | Pause Tone | 010xxxxx.mp3 |

Note that with the hex structure of the serial addressing.. 10 becomes 0A, and so forth until you hit 16.

### Link to OBS ###

There is a serial output in the arduino script that pipes to the wireless USB serial transmitter. pyserial script with serial receiver converts the output to a webpage for consumption in OBS.

#### Timer Output ####

Serial output prints out the time each second, and is printed out on the HTML page. If there is no data, the page will clear data after 10 seconds.

#### Tap Out Output ####

Serial output prints out the string 'redtapout' or 'bluetapout' upon the tapout button being pressed during fight. That string is interpreted to output a colored rectangle with the proper text indicating that said square has tapped out. Data clears after 10 seconds of no input.

### Timer Breakdown ###

Clock starts in standby. Time is set via command box. There are 2, 3, and 5 minute buttons, with indicator lamps.
In addition, there is a start, pause, and stop button. Pause button currently pauses clock, and needs to be pressed again to continue clock countdown.  
In standby both players need to press their color button to indicate that they are ready. Sounds and lights indicate that they are ready.
StartTree kicks in, produces the start tones, then starts clock.
Either Red or Blue button can be pressed for tap out during fight, which also includes pause currently.

#### Color Defining ####

##### NeoPixel #####
Status color changes are set for the clock when the clock passes 1 minute left, and 10 seconds left. In addtion there is a color change on pause.

Colors variables are defined as decimal for the clock (Adafruit NeoPixel codebase) you can find an online converter to take hex and convert to said color.

##### DMX Lighting ######

Everything is a value between 0-255. See each DMX light's manuals for marking which channel does what action and at what value if needed.

| DMX Command | What it does |
| setChannelValue (channel, value) | Sets one individual channel to a specific value between 0-255 |
| setChannelRange (starting channel, ending channel, value) | Sets a range between two channels (inc'l start and end), and each channel get's set to a specifc value between 0-255) |

## Current items to change ##

- Bonus wish: adding button tap to say you are ready upon pause
- Start tree should stay green when match has started, but not interfere with clock start.
- Working on pulsing the clock on pause, and possibly squares when ready.
- Serial output to OBS should also include an indicator for player ready for both squares, then stop when fight starts.