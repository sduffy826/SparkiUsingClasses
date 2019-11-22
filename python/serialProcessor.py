import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats


useBluetooth = False
isIBMMacBook = False
# DEBUGGING    = False
#TAPEWIDTH    = 4.7
#MINTAPEWIDTH = TAPEWIDTH - 0.5
#MAXTAPEWIDTH = TAPEWIDTH + 0.5

serialLogFile = "serialLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"
pickleFile    = "pickFile_withSensors.bin" 

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14601', baudrate=19200)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=9600)

readLines = 0
runTime   = 60           # Only runs for 2 minutes
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b'Trigger')    # Push something on the serial port, this will activate it

logFileHandle = open(serialLogFile,"at") # Append and text file
currTime      = time.time() - startTime
leaveLoop     = False

# Init to values that you know are outside range of min/maxe's you'll see
#worldXMin = 200.0
#worldXMax = -200.0
#worldYMin = 200.0
#worldYMax = -200.0

#pathsVisited   = []
#paths2Visit    = []
#goalPosition   = {}  # Pose of the goal
#potentialGoals = [] # List of potential goals
#startPosition  = {}

#pathValueList = []
#errorList     = []

separator = '-' * 80  # Give 80 -'s (cool)
asterisk  = '*' * 80
## NEED TO MAKE CHANGES TO POPULATE THE LISTS/DICTS ABOVE 
while ((currTime) < runTime) and (leaveLoop == False):
  try:
    stringFromSparki = ser.readline().decode('ascii').strip()  
    #time.sleep(0.5)
    
    logFileHandle.write(stringFromSparki + "\n")
    print("Time: {0} SerialFromSparki: {1}".format(currTime,stringFromSparki))
   
    if stringFromSparki.upper() == "IR,DONE":
      leaveLoop = True
    elif stringFromSparki.upper() == 'IR,PATHSTART' or stringFromSparki.upper() == 'IR,GOALSTART' or \
                                                                stringFromSparki.upper() == 'IR,GOTOSTART': 
      logFileHandle.write(separator + "\n")  # Mark start of path
      gv.pathValueList.clear()  # Clear arrays
      gv.errorList.clear()
    elif stringFromSparki.upper() == 'IR,PATHEND':
      sparkiStats.processValueList()

      sparkiStats.writeVariables()

      # !!!!!!!! PUT IN LOGIC HERE TO UPDATE MAP
      if len(gv.errorList) > 0:
        print(asterisk)
        logFileHandle.write(asterisk + "\n")
        print(" E R R O R S  processing this path, will redo it")
        logFileHandle.write(" E R R O R S  processing this path, will redo it\n")
        for anError in gv.errorList:
          print(anError)
          logFileHandle.write(str(anError) + "\n")
        print(asterisk)
        logFileHandle.write(asterisk + "\n")
      
    elif stringFromSparki.upper() == "IR,INS":      # Want instructions on where to go
      instructions2Send = sparkiStats.tellSparkiWhatToDo() + gv.SERIALTERMINATOR
      print("Sending this to sparki: {0}".format(instructions2Send))
      ser.write(instructions2Send.encode())
      time.sleep(0.05)
      ser.flush()
      time.sleep(0.5)
      # leaveLoop = True ## CHANGE THIS DOWN ROAD

    sparkiStats.setPathValueListFromString(stringFromSparki)
    if gv.DEBUGGING:
      print("len pathValueList: {0}".format(len(gv.pathValueList)))

  except:
    print("Exception raised in serialProcessor.py")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    leaveLoop = True

  readLines += 1
  currTime = time.time() - startTime

try:
  ser.flush() #flush the buffer
except:
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")

if gv.DEBUGGING:
  print("Read {0} lines, sensorList size: {1}".format(readLines, len(gv.pathValueList)))

  for aDictItem in gv.pathValueList:
    print(dict(aDictItem))

# Close the file with the strings
logFileHandle.close()  

# Save the list of dictionary items
pickle_out = open(pickleFile,"wb")
pickle.dump(gv.pathValueList, pickle_out)
pickle_out.close()

sparkiStats.writeVariables()
