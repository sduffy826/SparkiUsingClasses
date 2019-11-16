
from statistics import mean, median, mode, stdev
import os
import sys
import utilities
"""
Sample reading (put in breaks to make readable):

IR,StateChg,x,12.09,y,0.00,<,0,el,980,ll,973,lc,955,lr,915,er,980,
  ell,0,lll,0,lcl,0,lrl,0,erl,0,
  sdl,0,sdr,0,sol,0,sae,1,slp,0,srp,0,sel,0,ser,0,sas,0,sgl,0

"""

# Return a dictionary for the pose
def getPose(arrayValues, debugIt=False):
  if debugIt:
    print("in getPose, len of args: {0}".format(len(arrayValues)))
  poseToReturn = { "RECTYPE" : "POSE" }
  idx = 1
  while (idx < len(arrayValues)):
    if debugIt:
      print("getPose key: {0} value: {1}".format(arrayValues[idx],arrayValues[idx+1]))
    poseToReturn[arrayValues[idx]] = arrayValues[idx+1]
    if debugIt:
      print(poseToReturn)
    idx += 2
  
  if debugIt:
    print("poseToReturn: ")
    print(poseToReturn)
  return poseToReturn


# Return dictionary for the state
def getState(arrayValues, debugIt=False):
  idx = 2 # Have it represent where the value is not the label for it
  stateToReturn = { "RECTYPE" : "STATE" }
  while idx < len(arrayValues):
    sensorLabel = arrayValues[idx]
    sensorValue = arrayValues[idx+1]
    idx += 2
    stateToReturn[sensorLabel] = sensorValue
  return stateToReturn

# This routine processes the intersection values that are passed in; it should only
# have the values related to one side... i.e. there should be an array with leftValues
# and one for rightValues created by calling program... it then passes the values to
# this routine... it tells us the intersectionName (just informational) and the angle
# that we should use for the path at that intersection... i.e. left intersections
# will be -90' from the path orientation and right intersections will be +90'
# If the intersection is invalid it'll be written to the 'errorList' list, if it's
# good it'll be written to teh array 'paths2Visit'
def processIntersections(intersectionName, angle2Adjust, arrayOfIntersections):
  tempX = -200
  tempY = -200
  for pathDict in arrayOfIntersections:
    if pathDict["TYPE"] = "START":
      tempX = pathDict["x1"]
      tempY = pathDict["y1"]
    else:
      # Calculate the width of the line, it's pathDicts[x1,y1] - tempx,tempy
      # if it's outside threshold then report this as an error
      widthOfLine = utilities.calculateDistanceBetweenPoints(tempX, tempY, pathDict["x1"], pathDict["y1"])
      if (widthOfLine < (TAPEWIDTH - .5)) or (widthOfLine > (TAPEWIDTH + 0.5)):
        errorDict = { "x1" : tempX, "y1" : tempY, "<" : pathDict["<"], "x2" : pathDict["x1"], "y2": pathDict["y1"] }
        errorDict["TYPE"] = intersectionName
        errorDict["MESSAGE"] = "Invalid date width: {0}".format(widthOfLine)
        errorList.append(errorDict)
      else:
        # Good record
        midPointOfTape = utilities.getMidpointBetweenPoints(tempX, tempY, pathDict["x1"], pathDict["y1"])
        angle2Travel = utilities.getAngleAfterAdjustment(pathDict["<"],angle2Adjust)
        infoMsg = intersectionName + " intersection"
        path2Travel = { "x1" : midPointOfTape[0], "y1" : midPointOfTape[1], "<" : angle2Travel, "INFO" : infoMsg }
        paths2Visit.append(path2Travel)
      # Reset tempx, tempy
      tempX = -200
      tempY = -200

# This is called when we've read the 'start' 'end' block of data from the sparki
# The block of data sent represents a line segment, the values are stored in
# array 'pathValueList', each row in that is a dictionary item
# State legend: sas-EntranceOfMaze, sol-OnALine, sae-AtExit, slp-StartLeftPath,
#               srp-StartRightPath, sel-EndLeftPath, ser-EndRightPath, sgl-Goal
def processValueList():
  initValues = True
  startX = 0
  startY = 0
  endX = 0
  endY = 0
  leftIntersections = []
  rightIntersections = []
  for dictItem in pathValueList:
    recPose = { "x1" : dictItem["x"], "y1" : dictItem["y"], "<" : dictItem["<"] }
    # If need to initialize or "sas" (At start of maze) then set values
    if initValues == True or dictItem["sas"] == 1:
      segmentPose = recPose.copy()
      startX = dictItem["x"]
      startY = dictItem["y"]
      endX   = startX
      endY   = startY 
      initValues = False
    else:
      endX = dictItem["x"]
      endY = dictItem["y"]

    # If see 'At Start' of maze (and not seen before) set variable
    if dictItem["sas"] == 1:
      if len(startPosition) == 0:
        startPosition = recPose.copy()

    # At exit, add this to the list of potential goals
    if dictItem["sae"] == 1:
      potentialGoals.append(recPose)

    # If it is a goal then set the goal position to this
    if dictItem["sgl"] == 1:
      goalPosition = recPose

    # Start or end of the left path, put into array for later processing
    if dictItem["slp"] == 1 or dictItem["sel"] == 1:
      tempDict = recPose.copy()
      if dictItem["slp"] == 1:  # left path start
        tempDict["TYPE"] = "START"
      else:
        tempDict["TYPE"] = "END
      leftIntersections.append(tempDict)

    # If processing right path record then load into array for processing later
    if dictItem["srp"] == 1 or dictItem["ser"] == 1:
      tempDict = recPose.copy()
      if dictItem["srp"] == 1: # right path start
        tempDict["TYPE"] = "START"
      else:
        tempDict["TYPE"] = "END"
      rightIntersections.append(tempDict)
  
  # We have the segment visited; put it into pathsVisited
  if initValues == False:  # If true we didn't process data above
    segmentPose["x2"] = endX
    segmentPose["y2"] = endY
    pathsVisited.append(segmentPose)
  
    # Recalculate the max min values so far
    worldXMin = min(worldXMin, startX, endX)
    worldXMax = max(worldXMax, startX, endX)
    worldYMin = min(worldYMin, startY, endY)
    worldYMax = max(worldYMax, startY, endY)

    processIntersection("Left",-90,leftIntersections)
    processIntersection("Right",90,rightIntersections)

def setVars(sensorList, stringFromSparki, debugIt=False):
  # Store values, we care about StateChg and POses, we'll make them
  #   dictionary objects and put them into the sensorList array
  arrayOfValues = stringFromSparki.split(",")
  if len(arrayOfValues) > 2:  # Has to has more than two values
    keyWord = arrayOfValues[0]
    if keyWord == "IR" and arrayOfValues[1] == "StateChg":
      if debugIt:
        print("in IR processing")
      stateObj = getState(arrayOfValues,debugIt)
      sensorList.append(stateObj)
    
    if keyWord == "PO":  # This is still here but I moved the pose info into the IR record so 
      if debugIt:        #   it's probably not needed.... left in case :)
        print("In POSE processing")
      logPose = getPose(arrayOfValues,debugIt)
      sensorList.append(logPose)
      if debugIt:
        print(logPose)
    if debugIt:
      print("Leaving setVars, len of array {0}".format(len(sensorList)))

