import serial
import time

port = ''  #enter the path to whatever port you're using here
baudRate = 9600 #enter whatever baud rate is being used on the arduino

ser = serial.Serial(port, baudRate, timeout = 1.0)
time.sleep(3)  #Arduino restarts when port is opened, this sleep is so that no data is lost during the restart
