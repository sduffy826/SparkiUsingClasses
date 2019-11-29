import serial
import time
from datetime import datetime
import sys
import os 

useBluetooth = False
isIBMMacBook = True

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14601', baudrate=19200)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=9600)

runTime   = 10           # Limit run time
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b' ')          # Push something on the serial port, this will activate it

currTime   = time.time() - startTime
leaveLoop  = False

def send2Sparki(theString):
  theLen = len(theString)
  print("In send2Sparki, theLen: {0} string: {1}".format(theLen,theString))
  ser.write(chr(theLen).encode())
  time.sleep(0.05)
  ser.write(theString.encode())
  time.sleep(0.1)

print("Sparki should init communication")
while ((currTime) < runTime) and (leaveLoop == False):
  try:
    print("currTime {0}".format(currTime))
    stringFromSparki = ser.readline().decode('ascii').strip()
    print("stringFromSparki: {0}".format(stringFromSparki))
    time.sleep(1.2)
    if stringFromSparki.upper() == "DONE":
      leaveLoop = True
    else:
      send2Sparki("M,x,1.2,y,-3.45,<,67,X,x,89.01,y,23.45,<,359")
      time.sleep(0.2)
  except:
    print("in exception processing")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    leaveLoop = True

  currTime = time.time() - startTime

ser.flush() #flush the buffer