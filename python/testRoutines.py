import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats
import utilities


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

if True:
  theAngle = 0
  while theAngle <= 360:
    print("<: {0}, cos: {1}, sin{2}".format(theAngle,utilities.degreesCos(theAngle),utilities.degreesSin(theAngle)))
    theAngle += 45


# Test the sensor/robot pose calculation (i.e. where robot is based on sensor, and vica versa)
if True:
  testPose = testGetPose()
  robotPose = sparkiStats.getActualPoseOfSensorPose(testPose, True)
  print("robotPose {0}".format(robotPose))
  print("testPose {0}".format(testPose))
  forwardPose = sparkiStats.getActualPoseOfSensorPose(robotPose, False)
  print("forwardPose {0}".format(forwardPose))

if True:
  testLogFile()
  sparkiStats.writeHelper("Fluffy is a great chicken")
  gv.logFile.close()