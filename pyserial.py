## Requires pyserial to be installed to reference serial
## For changes to take effect, the script must be stopped, and started again. In addition, the fight will have to start for the clock changes to refresh in memory.
import serial
import time

file_name = "serial.html" # Once created, open this file in a browser. Reference this file in OBS

# Adapt serial port number & baud rate to your system.
serial_port = 'COM3'
baudrate = 38400

def write_page(data_list):
    fo = open(file_name,"w+")
    # Start of HTML page.
    fo.write("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>")
    fo.write("<meta http-equiv='refresh' content='1'>")
    fo.write("<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>")
    ## Background below should be transparent for OBS to pick up just the text, but OBS CSS can clean that up.
    fo.write("<p style='color:#FFFFFF;font-size: 120px;text-align: center;font-family: Impact'>") # p style needs a single quote around the entire set of parameters for it to work correctly.
    fo.write(data_str)
	

s = serial.Serial(serial_port,baudrate,timeout=10) # Open serial port.
s.dtr = 0 # Reset Arduino.
s.dtr = 1
print("Waiting for data...");
time.sleep(2) # Wait for Arduino to finish booting.
s.reset_input_buffer() # Delete any stale data.

while 1:
    data_str = s.readline().decode() # Read data & convert bytes to string type.
    print("Updating HTML...");
    if data_str is None:
        data_str=""
    #elif data_str=="redready":
    #    print("Red Ready");
    #    data_list = "<div style='width:100px;height:110px;font-size:76px;background-color:red;color:#FFFFFF;font-family: Impact;margin:0 auto;text-align:center'>Ready</div>"
    #    data_str = data_list
    #elif data_str=="blueready":
    #    print("Blue Ready");
    #    data_list = "<div style='width:100px;height:110px;font-size:76px;background-color:blue;color:#FFFFFF;font-family: Impact;margin:0 auto;text-align:center'>Ready</div>"
    #    data_str = data_list
    #elif data_str=="bothready":
    #    print("Both Ready");
    #    data_list = "<div style='width:100px;height:110px;font-size:76px;background-color:red;color:#FFFFFF;font-family: Impact;margin:0 auto;text-align:center'>Ready</div><div style='width:100px;height:110px;font-size:76px;background-color:blue;color:#FFFFFF;font-family: Impact;margin:0 auto;text-align:center'>Ready</div>"
    #    data_str = data_list
    elif data_str=="redtapout":
        print("Red Tap Out");
        data_list = "<div style='display: flex; max-width: 280px; height: 140px; border: 2px solid #2d2d2d; color: #ffffff; text-align: center; justify-content: center; align-items: center; font-family: Impact; background-color: red; font-size: 84px; font-weight: bold;'>TAP OUT</div>"
        data_str = data_list
    elif data_str=="bluetapout":
        print("Blue Tap Out");
        data_list = "<div style='display: flex; max-width: 280px; height: 140px; border: 2px solid #2d2d2d; color: #ffffff; text-align: center; justify-content: center; align-items: center; font-family: Impact; background-color: blue; font-size: 84px; font-weight: bold;'>TAP OUT</div>"
        data_str = data_list
    else:
        # Clean up input data.
        # Expected format: "$,id1,value1,id2,value2,...,CRLF"
        #data_str = data_str.replace(' ','') # Remove whitespace.
        #data_str = data_str.replace('\r','') # Remove return.
        #data_str = data_str.replace('\n','') # Remove new line.
        #data_str += '123,65,1,999,cpv,236' # Add some more data
        print(data_str)
        # Split data in fields separated by ','.
        data_list = data_str
        #del data_list[0] # Remove '$'

    # Write HTML page.
    write_page(data_list)