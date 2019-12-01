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

serialLogFile = "serialLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"
pickleFile    = "pickFile_withSensors.bin" 

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14601', baudrate=57600)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=9600)

readLines = 0
runTime   = 3600         # Run for 60 minutes
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b'Trigger')    # Push something on the serial port, this will activate it

gv.logFile = open(serialLogFile,"at") # Append and text file
currTime   = time.time() - startTime
leaveLoop  = False

separator = '-' * 80  # Give 80 -'s (cool)
asterisk  = '*' * 80
## NEED TO MAKE CHANGES TO POPULATE THE LISTS/DICTS ABOVE 

gv.currentMode = ' ' # Set current mode to unknown

# --------------------------------------------------------------------------------------------------
# Routine to handle start/stop instructions for goal, explore, goto... (not IR,DONE or IR,INS)
# it only returns 'True' when the calling program needs to stop; the parms are:
#  insStart - is boolean (if a start instruction, otherwise this is a stop instruction)
#  insMode - the mode of the instruction (i.e. C_EXPLORE)
#  insPose - The pose of the sensor at the time of start/stop (we'll convert)
#  insGoalFlag - a boolean indicator that's only applicable if the insMode is C_GOAL, in that
#                  case it's a boolean as to whether the goal was found (i.e. it had an obstacle
#                  in front of it)
def handleInstruction(isStart,insMode,insPose,insGoalFlag):
  flgStopMainline = False
  gv.currentMode  = insMode  # Update the global which has our current mode 
  msg = "Start" if isStart else "Stop"
  print("handleInstructions (HI) (sensorPose) :{0} :{1} :({2}) :{3}".format(msg,insMode,str(insPose),insGoalFlag))

  if isStart:
    # Save the pose the robot was at at the start of the instruction... we'll also set
    # gv.lastRobotDictItem to this location; we need that when eXploring as it'll be the
    # node that we're starting from during the exploration.
    gv.robotPoseFromInstruction = sparkiStats.getActualPoseOfSensorPose(insPose)
    robotNode = sparkiStats.getNodeAtPosition(gv.robotPoseFromInstruction["x"],gv.robotPoseFromInstruction["y"],False)
    if (robotNode != -1):
      gv.robotPoseFromInstruction["NODEID"] = robotNode
    
    gv.lastRobotDictItem = gv.robotPoseFromInstruction.copy()
    print(" ")
    print("HI: robotPoseFromInstruction: {0}".format(gv.robotPoseFromInstruction))
    print("HI: lastRobotDictItem: {0}".format(gv.lastRobotDictItem))
    print(" ")
    

    # At start we'll clear the pathValueList and errorList
    gv.pathValueList.clear()  # Clear arrays
    gv.errorList.clear()
   
    if insMode == gv.C_GOAL:
      pass
    elif insMode == gv.C_EXPLORE:
      pass
    elif insMode == gv.C_GOTO:
      pass
  else:
    if insMode == gv.C_GOAL:
      sparkiStats.checkAndSetGoalStatus(insGoalFlag)
    elif insMode == gv.C_EXPLORE:      
      sparkiStats.processValueList()
      if (gv.DEBUGGING == False):
        sparkiStats.writeVariables()  # For debugging
      # Should update map
      
      # If error then write them to console, if an error encountered on first explore
      # then terminate program, need restart
      if len(gv.errorList) > 0:
        print(asterisk)
        gv.logFile.write(asterisk + "\n")
        print(" E R R O R S  processing this path, will redo it")
        gv.logFile.write(" E R R O R S  processing this path, will redo it\n")
        for anError in gv.errorList:
          print(anError)
          gv.logFile.write(str(anError) + "\n")
        print(asterisk)
        gv.logFile.write(asterisk + "\n")
        if len(gv.nodeList) == 0:
          print("ERROR ON FIRST PATH, RESTART THE PROGRAM!!!")
          flgStopMainline = True
    elif insMode == gv.C_GOTO:
      pass
    # Since we're done processing the 'stop' let's clear variables
    gv.currentMode = ' '
   
  return flgStopMainline  

# --------------------------------------------------------------------------------------------------
# Checks if the instruction is a start or end state
def isSparkiStartStopInstruction(theInstruction2Check):
  # Format of start/end state is:
  # 0    1             2                           3  4  5  6  7  8 9  10  11
  # IR,INSTART|INSTOP,mode(explore,goal,goto,done),PO,x,n.n,y,m.m,<,l,OBST,T|F
  # The OBST is only on INSTOP records (T=True, F=False)
  dumArray = theInstruction2Check.upper().split(',')
  if (gv.DEBUGGING): print(str(dumArray))
  flgToRtn = False
  rtnDict  = {}
  if len(dumArray) >= 10:
    if dumArray[0] == "IR":
      if ((dumArray[1] == "INSSTART") or (dumArray[1] == "INSSTOP")):
        flgToRtn           = True
        rtnDict["ISSTART"] = (dumArray[1] == "INSSTART")
        rtnDict["MODE"]    = dumArray[2] # This is the value that matches C_EXPLORE, C_GOAL....
        rtnDict["POSE"]    = { "x" : float(dumArray[5]), "y" : float(dumArray[7]), "<" : int(dumArray[9])}
        if dumArray[1] == "INSSTART":
          rtnDict["GOAL"] = False 
        else:
          rtnDict["GOAL"] = (dumArray[11] == "T")
  return flgToRtn, rtnDict

# Utility method to send a string to the sparki
def send2Sparki(theString):
  theLen = len(theString)
  print("In send2Sparki, theLen: {0} string: {1}".format(theLen,theString))
  ser.write(chr(1).encode())  # this is the trigger for the start... this would never appear in data normally
  ser.write(chr(theLen).encode())
  time.sleep(0.05)
  ser.write(theString.encode())
  time.sleep(0.1)

# ==================================================================================================
# Mainline program
# ==================================================================================================
# Put out separators so it's easy to spot in the terminal
for sepIdx in range(20): print(" ")
for sepIdx in range(1,4):
  print(asterisk)

while True:
  print("Waiting for handshake from sparki")
  ser.write(b' ')  # Poke the port
  stringFromSparki = ser.readline().decode('ascii').strip()
  if stringFromSparki == "IR,HNDSHK":
    ser.write(("$"+gv.C_EXPLORE).encode())
    break
  else:
    print("Ignoring this from sparki: {0}".format(stringFromSparki))
print("Handshake complete, told sparki that we're in C_EXPLORE MODE  $$$CHANGE THIS$$ DOWN THE ROAD")

dumdog = 0

# Enter main loop now
while ((currTime < runTime) and (leaveLoop == False)):
  try:
    stringFromSparki = ser.readline().decode('ascii').strip()  
    gv.logFile.write(stringFromSparki + "\n")
    print("Time: {0} SerialFromSparki: {1}".format(currTime,stringFromSparki))
   
    # See if this is information from the sparki about start or stop instruction (this
    # is only for explore, goal, goto), the IR,DONE, IR,INS is also sent
    isAStartStopInstruction, dictOfInfo = isSparkiStartStopInstruction(stringFromSparki)
    if (isAStartStopInstruction):  # This handles path, goal, goto etc...
      print(separator)
      gv.logFile.write(separator + "\n")  # Mark start of path
      isStartIns = dictOfInfo["ISSTART"]
      insMode    = dictOfInfo["MODE"]
      insPose    = dictOfInfo["POSE"]
      insGoal    = dictOfInfo["GOAL"]

      # Routine below handles stop/start instructions, it only returns true when there's an 
      # error on the initial 'explore' path (means a restart)
      # The routine below also converts the sensorPose into the robot pose
      leaveLoop = handleInstruction(isStartIns,insMode,insPose,insGoal)
    elif stringFromSparki.upper() == "IR,DONE":
      print(asterisk)
      leaveLoop = True
    elif stringFromSparki.upper() == "IR,INS":      # Want instructions on where to go
      #shmoo = ser.readline().decode('ascii').strip()  
      #print("shmoo {0}".format(shmoo))
   
      print(separator)
      instructions2Send = sparkiStats.tellSparkiWhatToDo() #+ gv.SERIALTERMINATOR
      print("*****************")
      print("Sending this to sparki: {0}".format(instructions2Send))
      print("*****************")
      currTime = time.time() - startTime
      if ser.is_open:
        print("Time: {0}  Serial is open".format(currTime))
      else:
        print("Serial port is NOT open")

      send2Sparki(instructions2Send)

      currTime = time.time() - startTime
      print("Time: {0}  Instruction sent".format(currTime))
      #ser.write(b" ")
      #time.sleep(0.05)
      #ser.flush()
      #time.sleep(0.5)
    
    # If in 'explore' mode then we'll update the path value list (it's used to determine map), we
    # don't do that when in goal mode or 'goto' mode
    if gv.currentMode == gv.C_EXPLORE:
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

sparkiStats.writeVariables()

# Close the file with the strings
gv.logFile.close()  

# Save the list of dictionary items
pickle_out = open(pickleFile,"wb")
pickle.dump(gv.pathValueList, pickle_out)
pickle_out.close()
