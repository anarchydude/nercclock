#!/bin/bash
# This is if you need to run this on *Nix or macOS
# Pre-req..
# pipx install pyserial
# source like below to test..

# Activate the virtual environment first
source /Users/youruserhere/.local/pipx/venvs/pyserial/bin/activate

# Then run your Python script
python3 /Users/youruserhere/path/to/pyserial/pyserial.py "$@"
