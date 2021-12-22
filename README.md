# NERC Timer #

This is the codebase for the new NERC timer. The clock hub runs off a Mega 2560. This helps avoid interrupts when polling the timer.
The audio files must be in a specific format - 001xxxxx, 002xxxxx, etc.. That lies in the 01 folder, which is not hosted, as it is just music. Look for the references.

## Equipment Pre-reqs ##

- USB Serial Receiver for machine used to pull timer, and other outputs from the arduino.
- Python, pyserial on whichever OS you are running the clock script from.
- Heavy gauge DMX cable for clock DMX line. Will pull 5A at 5VDC. Needs to be short run.