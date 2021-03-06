import os
import time
from datetime import datetime
import math
from getkey import getkey, keys


# Constant for the maximum delta between two angles to still consider
# them to be in the same direction
MAXANGLEDELTASTOBECONSIDEREDSAME = 14

# ----------------------------------------------------------------------
# Helper to say that two angles are close enough (i.e. 88 and 91 are 
# pointing down the same path)
def areAnglesCloseEnough(angle1, angle2):
  return (abs(angle1 - angle2) <= MAXANGLEDELTASTOBECONSIDEREDSAME)  

# ----------------------------------------------------------------------
# Helper to return cosine of degree angle.
def degreesCos(theDegrees):
  return math.cos(math.radians(theDegrees))

# ----------------------------------------------------------------------
# Helper to return cosine of degree angle.
def degreesSin(theDegrees):
  return math.sin(math.radians(theDegrees))  

# ----------------------------------------------------------------------
# Takes in an angle and an adjustment (+ or -) and returns the resulting
# angle within 0->359' range
def getAngleAfterAdjustment(baseAngle, adjustment):
  return (360 + (baseAngle + adjustment))%360  

# ----------------------------------------------------------------------
# Calculate difference between two points 
def getDistanceBetweenPoints(x1,y1,x2,y2):
  deltaX = x2 - x1
  deltaY = y2 - y1
  return round(math.sqrt((deltaX*deltaX)+(deltaY*deltaY)),2)

# ----------------------------------------------------------------------
# Prompt for a key from the user
def getInputChar(message):
  print(message)
  return getkey()

# ----------------------------------------------------------------------
# Return the midpoint between two points (returns a tuple)
def getMidpointBetweenPoints(x1,y1,x2,y2):
  xMid = round(((x1 + x2)/2.0),2)
  yMid = round(((y1 + y2)/2.0),2)
  return ( xMid, yMid )

# ----------------------------------------------------------------------
# Archive a file by renaming it to have a suffix, if you don't pass in the
# suffix I'll use the iso date/time
def fileArchive(fileName,suffix=""):
  if len(suffix) == 0:
    suffix = datetime.now().isoformat(timespec='seconds').replace("-","").replace(":","")
  newName = fileName.strip() + "_" + suffix
  if fileExists(fileName):
    renameFile(fileName, newName)

# ----------------------------------------------------------------------
# Return the midpoint between two points (returns a tuple)
def fileExists(fileName):
  return os.path.isfile(fileName)

# ----------------------------------------------------------------------
# Return the midpoint between two points (returns a tuple)
def renameFile(oldName, newName):
  return os.rename(oldName, newName)  