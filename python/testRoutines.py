import os
import sys
import serial
import time
from datetime import datetime
import pickle
import csv

import sharedVars as gv
import sparkiStats
import utilities

"""
if gv.writeCSVData:
  if utilities.fileExists(gv.csvFileName):
    utilities.fileArchive(gv.csvFileName)

with open(gv.csvFileName, 'a') as csv_file:
  gv.csvWriter = csv.DictWriter(csv_file, fieldnames=gv.csvFieldNames)
  gv.csvWriter.writeheader()

#with open(gv.csvFileName, 'a') as csv_file:
#  gv.csvWriter = csv.DictWriter(csv_file, fieldnames=gv.csvFieldNames)

theRecord = {"x_value" : 2, "y_value" : 3}
gv.csvWriter.writerow(theRecord)
def writePose(stringFromSparki):    
  print("in writePose")
  if gv.writeCSVData:
    print("writePose1")
    dumArray = stringFromSparki.split(',')
    dataRec = { "x_value" : float(dumArray[2]),
               "y_value" : float(dumArray[4]) }
    print(dataRec)            
    print(type(gv.csvWriter))
    gv.csvWriter.writerow(dataRec)
    print("here")
"""
def writePose(stringFromSparki):  
 try:
  if gv.writeCSVData > 0:
    if gv.writeCSVData == 1:
      if utilities.fileExists(gv.csvFileName):
        utilities.fileArchive(gv.csvFileName)
  
      gv.csvFileHandle = open(gv.csvFileName,"w")
      gv.csvWriter     = csv.DictWriter(gv.csvFileHandle, fieldnames=gv.csvFieldNames)
      gv.csvWriter.writeheader()
      gv.csvFileHandle.close()
      gv.csvFileHandle = open(gv.csvFileName,"a")
      gv.csvWriter     = csv.DictWriter(gv.csvFileHandle, fieldnames=gv.csvFieldNames)
      gv.writeCSVData  = 2
    dumArray = stringFromSparki.split(',')
    dataRec = { "x_value" : float(dumArray[2]),
               "y_value" : float(dumArray[4]) }
    print(dataRec)            
    print(type(gv.csvWriter))
    gv.csvWriter.writerow(dataRec)
    print("here")
 except:
   print("Exception raised - getNextGoal2Visit")
   exc_type, exc_obj, exc_tb = sys.exc_info()
   fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
   print(exc_type, fname, exc_tb.tb_lineno)
   sys.exit()


def saveMapElementsToPickle(pickleFile):
  pickle_out = open(pickleFile,"wb") 
  pickle.dump([gv.worldXMin, gv.worldXMax, gv.worldYMin, gv.worldYMax, gv.startOfMaze, \
               gv.nodeList, gv.nodeConnectionList, gv.potentialGoalNodes, \
               gv.potentialGoalDicts, gv.pathsVisited], pickle_out)
  pickle_out.close()

def loadMapElementsFromPickle(pickleFile):
  pickle_out = open(pickleFile,"rb") 
  gv.worldXMin, gv.worldXMax, gv.worldYMin, gv.worldYMax, gv.startOfMaze, \
     gv.nodeList, gv.nodeConnectionList, gv.potentialGoalNodes, \
     gv.potentialGoalDicts, gv.pathsVisited = pickle.load(pickle_out)
  pickle_out.close()


def testLogFile():
  serialLogFile = "serialLog." + datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","") + ".csv"
  gv.logFile = open(serialLogFile,"at") # Append and text file

def testGetPose():
  stringFromSparki = "PO,x,13.71,y,-0.96,<,0"
  arrayOfValues    = stringFromSparki.split(",")
  logPose = sparkiStats.getPose(arrayOfValues)
  print(logPose)
  return logPose

# Simple show pose
if False:
  testGetPose()

if False:
  theAngle = 0
  while theAngle <= 360:
    print("<: {0}, cos: {1}, sin{2}".format(theAngle,utilities.degreesCos(theAngle),utilities.degreesSin(theAngle)))
    theAngle += 45


# Test the sensor/robot pose calculation (i.e. where robot is based on sensor, and vica versa)
if False:
  testPose = testGetPose()
  robotPose = sparkiStats.getActualPoseOfSensorPose(testPose, True)
  print("robotPose {0}".format(robotPose))
  print("testPose {0}".format(testPose))
  forwardPose = sparkiStats.getActualPoseOfSensorPose(robotPose, False)
  print("forwardPose {0}".format(forwardPose))

if False:
  testLogFile()
  sparkiStats.writeHelper("Fluffy is a great chicken")
  gv.logFile.close()

if False:
  gv.worldXMin = 1.2
  gv.worldYMin = 2.3
  gv.nodeList.append(2)
  gv.nodeList.append(4)
  gv.potentialGoalNodes.append(3)
  gv.potentialGoalNodes.append(5)
  gv.potentialGoalNodes.append(6)
  mapPickle = "pickleTest1.bin"
  sparkiStats.zSaveMapElementsToPickle(mapPickle)
  gv.nodeList.clear()
  gv.potentialGoalNodes.clear()
  print(str(gv.nodeList))
  print(str(gv.potentialGoalNodes))
  sparkiStats.zLoadMapElementsFromPickle(mapPickle)
  print(str(gv.nodeList))
  print(str(gv.potentialGoalNodes))

  for i in range(2):
    if utilities.fileExists(mapPickle):
      print(mapPickle + " exists")
      utilities.fileArchive(mapPickle)
    else:
      print(mapPickle + " does not exist")

if False:
  ins = "M,x,5.32,y,0.0,<,-1,M,x,39.98,y,0.0,<,-1,M,x,40.14,y,18.06,<,-1,M,x,14.8,y,17.7,<,-1,M,x,14.86,y,26.33,<,-1,G,x,14.86,y,26.33,<,90"
  print(sparkiStats.tellSparkiHelper(ins))
  print("gv.pendingInstructions: {0}".format(gv.pendingInstructions))
  print(sparkiStats.tellSparkiHelper(""))

if True:
  string = "PO,x,12.3,y,45.6,<,10" 
  writePose(string)
  writePose(string)




"""
def testLoadPick(pickleFile):
  pickle_out = open(pickleFile,"rb") 
  gv.worldXMin, gv.worldXMax, gv.worldYMin, gv.worldYMax, gv.startOfMaze, \
     gv.nodeList, gv.nodeConnectionList, gv.potentialGoalNodes, \
     gv.potentialGoalDicts, gv.pathsVisited = pickle.load(pickle_out)
  pickle_out.close()
"""  