'''
Objectives:
    Arduino:    
        continuously read and recieve data
        store in respective EEPROM address

    Raspberry pi:
        same thing, read and recieve data
        develop the protocol and format
        recieves: all live data(temp,hum,ppm,soilMoisture)
        sends: threshold values(temp,humidity, AQ, plant name, NPK)
'''

'''
Pseudocode:

loop
    check-for-data-from-api() [getAPI()]
    if-data-recieved:
        send-to-arduino()  [onDataRecv()]
    Recieve-data-from-arduino()
    if-data-recieved:
        send-data-to-server/api() [postAPI()]

'''

import serial
import time

presetplant = ''
def getAPI():  #this one's your job, saalim. whatever you get, add it to the
    presetplant = 'placeholderium valuesera'
    return

def onDataRecv():  #Make it do the thing, future sufi
    return

def postAPI():
    return

port = ''  #enter the path to whatever port you're using here
baudRate = 9600 #enter whatever baud rate is being used on the arduino

ser = serial.Serial(port, baudRate, timeout = 1.0)
time.sleep(3)  #Arduino restarts when port is opened, this sleep is so that no data is lost during the restart
ser.reset_input_buffer()
print("Serial Initialised Successfully")



try:
    while True:
        time.sleep(0.01)
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8')
            print(line)
            onDataRecv()
except KeyboardInterrupt:  #change this to however you want to close the port
    print('closing serial port')
    ser.close()



# def sendData(values):
#     try:
#         ser.write(values'/n'.encode('utf-8'))



ser.close()
