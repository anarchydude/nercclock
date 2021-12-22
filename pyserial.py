# Requires pyserial to be installed to reference serial
import serial
import time

file_name = "serial.html" # Once created, open this file in a browser.

# Adapt serial port nr. & baud rate to your system. Currently on the stream box, COM3 is the receiver.
serial_port = 'COM3'
baudrate = 38400

def write_page(data_list):
    fo = open(file_name,"w+")
	# Start of HTML page.
    fo.write("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>")
    fo.write("<meta http-equiv='refresh' content='1'>")
    fo.write("<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>")
    fo.write("<body style='background-color:#000000;'>")
    #fo.write("<p style='color:#FFFFFF';font-size: 40px>")
    fo.write("<font size='6' color='#ffffff'>")
    fo.write(data_str)
	

s = serial.Serial(serial_port,baudrate) # Open serial port.
s.dtr = 0 # Reset Arduino.
s.dtr = 1
print("Waiting for data...");
time.sleep(2) # Wait for Arduino to finish booting.
s.reset_input_buffer() # Delete any stale data.

while 1:
    data_str = s.readline().decode() # Read data & convert bytes to string type.
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