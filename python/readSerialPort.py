import serial
import time
from datetime import datetime

useBluetooth = False
isIBMMacBook = True

outputFile   = "sparkiLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14601', baudrate=19200)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=9600)

readLines = 0
runTime   = 60          # Only runs for 2 minutes
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b'Trigger|')    # Push something on the serial port, this will activate it

fileHandle = open(outputFile,"at") # Append and text file
currTime   = time.time() - startTime
leaveLoop  = False

while ((currTime) < runTime) and (leaveLoop == False):
  try:
    stringFromSparki = ser.readline().decode('ascii').strip() 
    print("stringFromSparki: {0}".format(stringFromSparki))
    if stringFromSparki.upper() == "DONE":
      leaveLoop = True
    else:
      ser.write("Hi there at {0}|".format(time.time()).encode())
    fileHandle.write(stringFromSparki + "\n")
    print("Time: {0} SerialFromSparki: {1}".format(currTime,stringFromSparki))
  except:
    print("in exception processing")
  readLines += 1
  currTime = time.time() - startTime

ser.flush() #flush the buffer

print("Read {0} lines".format(readLines))

fileHandle.close()  