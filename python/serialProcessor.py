import serial
import time
from datetime import datetime
import sparkiStats

useBluetooth = False
isIBMMacBook = False

outputFile   = "serialLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14601', baudrate=19200)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=9600)

readLines = 0
runTime   = 60           # Only runs for 2 minutes
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b'Trigger')    # Push something on the serial port, this will activate it

sensorValueList = []

fileHandle = open(outputFile,"at") # Append and text file
currTime   = time.time() - startTime
leaveLoop  = False

while ((currTime) < runTime) and (leaveLoop == False):
  try:
    stringFromSparki = ser.readline().decode('ascii').strip()  
    if stringFromSparki.upper() == "DONE":
      leaveLoop = True

    fileHandle.write(stringFromSparki + "\n")
    print("Time: {0} SerialFromSparki: {1}".format(currTime,stringFromSparki))
    
    sparkiStats.setVars(sensorValueList,stringFromSparki)
  except:
    pass
  readLines += 1
  currTime = time.time() - startTime

ser.flush() #flush the buffer

print("Read {0} lines, sensorList size: {1}".format(readLines, len(sensorValueList)))

for aDictItem in sensorValueList:
  print(dict(aDictItem))

fileHandle.close()  