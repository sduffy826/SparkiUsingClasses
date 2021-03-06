import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats
import utilities
import csv

useBluetooth = False
isIBMMacBook = False

serialLogFile = "serialLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"

if useBluetooth == False:
  if isIBMMacBook:
    ser = serial.Serial(port='/dev/cu.usbmodem14401', baudrate=57600)
  else:
    ser = serial.Serial(port='/dev/cu.usbmodem1411', baudrate=19200)

readLines = 0
runTime   = 3600         # Run for 60 minutes
startTime = time.time()  # Returns time in seconds since epoch
ser.write(b' ')    # Push something on the serial port, this will activate it

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
  print("  (HI) robotPose: {0}".format(sparkiStats.getActualPoseOfSensorPose(insPose)))
  if isStart:
    # Save the pose the robot was at at the start of the instruction... we'll also set
    # gv.lastRobotDictItem to this location; we need that when eXploring as it'll be the
    # node that we're starting from during the exploration.
    # NOTE: If this is a pose instruction then lastRobotDictItem was already set, use it (that pose
    #       is sensor pose which makes sense)
    if insMode == "P":  # If this is a POSE instruction then use lastRobotDictItem
      gv.robotPoseFromInstruction = gv.lastRobotDictItem.copy()
      robotNode                   = gv.lastRobotDictItem["NODEID"]
    else:
      gv.robotPoseFromInstruction = sparkiStats.getActualPoseOfSensorPose(insPose)
      robotNode = sparkiStats.getNodeAtPosition(gv.robotPoseFromInstruction["x"],gv.robotPoseFromInstruction["y"],False)
    
    if (robotNode != -1):
      gv.robotPoseFromInstruction["NODEID"] = robotNode
    
    gv.lastRobotDictItem = gv.robotPoseFromInstruction.copy()
    print("  (HI): robotPoseFromInstruction: {0}".format(gv.robotPoseFromInstruction))
    print("  (HI): lastRobotDictItem: {0}".format(gv.lastRobotDictItem))
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
# Little helper to put out a message to the right of the string from the sparki... it helps
# when reviewing the log
def infoAboutString(stringThatSparkiSent):
  rtnMsg = ""
  dumArray = stringThatSparkiSent.upper().split(',')
  if len(dumArray) >= 10:
    if dumArray[0] == "IR":
      if (dumArray[1] == "INSSTART"):
        rtnMsg = "(already moving, sensor pose)"
      elif (dumArray[1] == "INSSTOP"):
        rtnMsg = "(not moving, sensor pose)"
      elif (dumArray[1] == "INS"):
        rtnMsg = "(need input :))"
      else:
        rtnMsg = "(u know this)"
  return rtnMsg

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
          print(" ") # Skip a line
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
  #time.sleep(0.05)
  ser.write(theString.encode())
  time.sleep(0.1)

def writePose(stringFromSparki):  
  try:
    if gv.writeCSVData > 0:
      if gv.writeCSVData == 1:  # First time call... create output file with header
        if utilities.fileExists(gv.csvFileName):
          utilities.fileArchive(gv.csvFileName)
    
        gv.csvFileHandle = open(gv.csvFileName,"w")
        gv.csvWriter     = csv.DictWriter(gv.csvFileHandle, fieldnames=gv.csvFieldNames)
        gv.csvWriter.writeheader()
        #gv.csvFileHandle.close()
        #gv.csvFileHandle = open(gv.csvFileName,"a")
        #gv.csvWriter     = csv.DictWriter(gv.csvFileHandle, fieldnames=gv.csvFieldNames)
        gv.writeCSVData  = 2

      dumArray = stringFromSparki.split(',')
      dataRec = { "x_value" : float(dumArray[2]),
                  "y_value" : float(dumArray[4]) }
      #print(dataRec)            
      #print(type(gv.csvWriter))
    gv.csvWriter.writerow(dataRec)
    gv.csvFileHandle.flush()
  except:
    print("Exception raised - writePose")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()
      
# ==================================================================================================
# Mainline program
# ==================================================================================================
# Put out separators so it's easy to spot in the terminal
for sepIdx in range(20): print(" ")
for sepIdx in range(1,4):
  print(asterisk)

gv.originalMode = 'X'  # Default mode to eXplore
resetRobotPose  = False # If on we'll tell robot what his pose is on first instruction

# If we have a map saved already ask the user which way they want to run it 
# eXplore (find map) or Goal mode where we start searching for goals
if utilities.fileExists(gv.pickleWithMap):
  sparkiStats.writeSpeech("Enter MODE")
  theResp = " "
  while ((theResp != "X") and (theResp != "G")):
    theResp = utilities.getInputChar("Enter mode: e'X'plore or 'G'oal").upper()
  if theResp == "X":  # They want new exploration... save old files
    utilities.fileArchive(gv.pickleWithMap)
    utilities.fileArchive(gv.pickleWithGoal)
  else:
    gv.originalMode = theResp
    resetRobotPose  = True
    sparkiStats.zLoadMapElementsFromPickle()
    sparkiStats.zLoadGoalElementFromPickle()
"""
if gv.writeCSVData:
  if utilities.fileExists(gv.csvFileName):
    utilities.fileArchive(gv.csvFileName)
  with open(gv.csvFileName, 'w') as csv_file:
    gv.csvWriter = csv.DictWriter(csv_file, fieldnames=gv.csvFieldNames)
    gv.csvWriter.writeheader()
  with open(gv.csvFileName, 'a') as csv_file:
    gv.csvWriter = csv.DictWriter(csv_file, fieldnames=gv.csvFieldNames)
"""
sparkiStats.writeSpeech("Awaiting handshake")
while True:
  print("Waiting for handshake from sparki")
  #ser.write(b' ')  # Poke the port
  stringFromSparki = ser.readline().decode('ascii').strip()
  if stringFromSparki == "IR,HNDSHK":
    ser.write(("$"+gv.originalMode).encode())
    break
  else:
    print("Ignoring this from sparki: {0}".format(stringFromSparki))

print("Handshake complete, told sparki that we're in {0} mode".format(gv.originalMode))

# Enter main loop now
while ((currTime < runTime) and (leaveLoop == False)):
  try:
    ignoreString     = False 
    isPoseForMapping = False
    stringFromSparki = ser.readline().decode('ascii').strip()  
    if ((len(stringFromSparki) > 1) and (stringFromSparki[0:2] == "AP")):
      sparkiStats.writeSpeech("Adjust Pose")
      ignoreString = True
    elif ((len(stringFromSparki) > 3) and (stringFromSparki[0:3] == "IP,")):
      isPoseForMapping = True
    else:
      gv.logFile.write(stringFromSparki + "\n")
      suffixMsg = infoAboutString(stringFromSparki)
      print("Time: {0} SerialFromSparki: {1} info:{2}".format(currTime,stringFromSparki,suffixMsg))
   
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
    elif ignoreString:
      pass
    elif isPoseForMapping:  # Just data for mapping the world, we write it to file for mapping
      writePose(stringFromSparki)
    elif stringFromSparki.upper() == "IR,DONE":
      print(asterisk)
      leaveLoop = True
      continue
    elif stringFromSparki.upper() == "IR,INS":      # Want instructions on where to go
      #shmoo = ser.readline().decode('ascii').strip()  
      #print("shmoo {0}".format(shmoo))
      print(separator)
      if resetRobotPose:
        sparkiStats.writeSpeech("Resetting Starting Pose")
        # On first instruction we need to tell robot what his pose is
        # The 'startOfMaze' has the pose of the sensor... the start always has that which
        # makes sense when u think about it... we need to get the robot pose for that
        # position
        newPose = sparkiStats.getActualPoseOfSensorPose(gv.startOfMaze)
        instructions2Send = gv.C_SETPOSE + ",x," + str(newPose["x"]) + \
                                           ",y," + str(newPose["y"]) + \
                                           ",<," + str(newPose["<"])
        gv.lastRobotDictItem = gv.startOfMaze.copy() # Set the robot
        resetRobotPose = False
      else:  
        instructions2Send = sparkiStats.tellSparkiWhatToDo() #+ gv.SERIALTERMINATOR
      
      print("*****************")
      print("Sending this to sparki: {0}".format(instructions2Send))
      print("*****************")
      currTime = time.time() - startTime
      if (gv.DEBUGGING):
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
    if ((gv.currentMode == gv.C_EXPLORE) and (isPoseForMapping == False) and (ignoreString == False)):
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

if gv.writeCSVData > 1:
  gv.csvFileHandle.close()  

if gv.writeSpeechFile > 1:
  gv.speechFileHandle.close()
