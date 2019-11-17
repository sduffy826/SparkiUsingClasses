
from statistics import mean, median, mode, stdev
import os
import sys

import sharedVars as gv
import utilities
"""
Sample reading (put in breaks to make readable):

IR,StateChg,x,12.09,y,0.00,<,0,el,980,ll,973,lc,955,lr,915,er,980,
  ell,0,lll,0,lcl,0,lrl,0,erl,0,
  sdl,0,sdr,0,sol,0,sae,1,slp,0,srp,0,sel,0,ser,0,sas,0,sgl,0


nodeList = [] # List of nodes
nodeConnectionList = [] # Nodes and node there connected, with angle and distance between them
nodesToVisit = [] # Nodes to visit and the angle they should visit
"""

# Return the node that is at this position (or within TAPEWIDTH away from it), it not found
# then a new node will be created (if last arg is true)
def getNodeAtPosition(x, y, createIfNonExistant=True):
  nodeIdToReturn = -1
  for idx in range(len(gv.nodeList)):
    if utilities.getDistanceBetweenPoints(x,y,gv.nodeList[idx][0],gv.nodeList[idx][1]) <= gv.TAPEWIDTH:
      nodeIdToReturn = idx
  if nodeIdToReturn == -1 and createIfNonExistant: # Not found, add it
    nodeIdToReturn = len(gv.nodeList) + 1
    gv.nodeList.append((x,y))
  return nodeIdToReturn



# --------------------------------------------------------------------------------------------------
# Return a dictionary for the pose, NOTE we convert the string numbers to correct representation in
# here.
def getPose(arrayValues):
  if gv.DEBUGGING:
    print("in getPose, len of args: {0}".format(len(arrayValues)))
  poseToReturn = { "RECTYPE" : "POSE" }
  idx = 1
  while (idx < len(arrayValues)):
    if gv.DEBUGGING:
      print("getPose key: {0} value: {1}".format(arrayValues[idx],arrayValues[idx+1]))
    if arrayValues[idx] == "x" or arrayValues == "y":
      theValu = float(arrayValues[idx+1])
    else:
      theValu = int(arrayValues[idx+1])
    poseToReturn[arrayValues[idx]] = theValu
    if gv.DEBUGGING:
      print(poseToReturn)
    idx += 2
  
  if gv.DEBUGGING:
    print("poseToReturn: ")
    print(poseToReturn)
  return poseToReturn

# --------------------------------------------------------------------------------------------------
# Return dictionary for the state, this converts the string 'numeric' values to their corresponding
# types (x and y are floats, the rest are ints)
def getState(arrayValues):
  idx = 2 # Have it represent where the value is not the label for it
  stateToReturn = { "RECTYPE" : "STATE" }
  while idx < len(arrayValues):
    sensorLabel = arrayValues[idx]
    if sensorLabel == "x" or sensorLabel == "y":
      sensorValue = float(arrayValues[idx+1])
    else:
      sensorValue = int(arrayValues[idx+1])
    idx += 2
    stateToReturn[sensorLabel] = sensorValue
  return stateToReturn

# --------------------------------------------------------------------------------------------------
# This routine processes the intersection values that are passed in; it should only
# have the values related to one side... i.e. there should be an array with leftValues
# and one for rightValues created by calling program... it then passes the values to
# this routine... it tells us the intersectionName (just informational) and the angle
# that we should use for the path at that intersection... i.e. left intersections
# will be -90' from the path orientation and right intersections will be +90'
# If the intersection is invalid it'll be written to the 'errorList' list, if it's
# good it'll be written to teh array 'paths2Visit'
def processIntersections(intersectionName, angle2Adjust, arrayOfIntersections):
  tempX    = -200
  tempY    = -200
  gotStart = False
  infoMsg  = intersectionName + " intersection"
  for pathDict in arrayOfIntersections:
    if pathDict["TYPE"] == "START":
      tempX    = pathDict["x1"]
      tempY    = pathDict["y1"]
      gotStart = True
    else:
      gotStart = False # We're not on a 'start' identifier
      # Calculate the width of the line, it's pathDicts[x1,y1] - tempx,tempy
      # if it's outside threshold then report this as an error
      widthOfLine = utilities.getDistanceBetweenPoints(tempX, tempY, pathDict["x1"], pathDict["y1"])
      if (widthOfLine < gv.MINTAPEWIDTH) or (widthOfLine > gv.MAXTAPEWIDTH):
        errorDict = { "x1" : tempX, "y1" : tempY, "<" : pathDict["<"], "x2" : pathDict["x1"], "y2": pathDict["y1"] }
        errorDict["TYPE"]    = infoMsg
        errorDict["MESSAGE"] = "Invalid date width: {0}".format(widthOfLine)
        gv.errorList.append(errorDict)
      else:
        # Good record
        midPointOfTape = utilities.getMidpointBetweenPoints(tempX, tempY, pathDict["x1"], pathDict["y1"])
        angle2Travel   = utilities.getAngleAfterAdjustment(pathDict["<"],angle2Adjust)
        path2Travel    = { "x1" : midPointOfTape[0], "y1" : midPointOfTape[1], "<" : angle2Travel, 
                           "TYPE" : "Intersection", "INFO" : infoMsg }
        gv.paths2Visit.append(path2Travel)
      # Reset tempx, tempy
      tempX = -200
      tempY = -200
  if gotStart:
    errorDict = { "x1" : tempX, "y1" : tempY, "<" : pathDict["<"] }
    errorDict["TYPE"]    = infoMsg
    errorDict["MESSAGE"] = "Got StartOfIntersection with no EndOfIntersection"
    gv.errorList.append(errorDict)

# --------------------------------------------------------------------------------------------------    
# This is called when we've read the 'start' 'end' block of data from the sparki
# The block of data sent represents a line segment, the values are stored in
# array 'pathValueList', each row in that is a dictionary item
# State legend: sas-EntranceOfMaze, sol-OnALine, sae-AtExit, slp-StartLeftPath,
#               srp-StartRightPath, sel-EndLeftPath, ser-EndRightPath, sgl-Goal

def processValueHelper(typeRecord, dictItem, list2Add2):
  tempPose         = dictItem.copy()
  tempPose["TYPE"] = typeRecord
  list2Add2.append(tempPose)

def processValueErrorHelper(typeRecord, dictItem, errorList2Add2, errMessage):
  tempPose          = dictItem.copy()
  tempPose["TYPE"]  = typeRecord
  tempPose["ERROR"] = errMessage
  errorList2Add2.append(tempPose)

# Little helper to return distance between two node objects
def getDistanceBetweenNodes(node1, node2):
  return utilities.getDistanceBetweenPoints(node1[0], node1[1], node2[0], node2[1])

# Little helper routine, this adds a dictionary item to the nodeConnectionList, it's
# in format { "id1" : nodeId1, "<" : angleFromID1toID2, "id2" :nodeId2, "len" : distanceBetweenThem}
def nodeConnectionHelper(lastDict,currDict):
  # Write routine to add these two nodes as connected into the node connection list
  distanceBetweenPts = utilities.getDistanceBetweenPoints(lastDict["x"],lastDict["y"],
                                                          currDict["x"],currDict["y"])
  nodeConnection = { "id1" : lastDict["NODEID"],
                     "<"   : lastDict["<"],
                     "id2" : currDict["NODEID"],
                     "len" : distanceBetweenPts }
  gv.nodeConnectionList.append(nodeConnection)

def processValueList():
 try:
  initValues = True
  tempList   = []
  tempError  = []
  leftFlag   = False
  rightFlag  = False
  
  for dictItem in gv.pathValueList:
    recPose = { "x" : dictItem["x"], "y" : dictItem["y"], "<" : dictItem["<"] }
    
    if initValues: # Save first record just in case we have to do a doover
      firstPose = recPose.copy()

    if dictItem["sas"] == 1:
      processValueHelper("sas",recPose,tempList)
    
    if dictItem["sae"] == 1:
      processValueHelper("sae",recPose,tempList)
    
    if dictItem["sgl"] == 1:
      processValueHelper("sgl",recPose,tempList)
    
    # Start of left intersection
    if dictItem["slp"] == 1:
      if leftFlag: # We seeing a new start without closing old end
        processValueErrorHelper("slp",recPose,tempError,"Open slp record")
      else:
        leftFlag     = True
        leftDictItem = recPose.copy()

    # End of left intersection
    if dictItem["sel"] == 1:
      if leftFlag == False:  # didn't see start of intersection
        processValueErrorHelper("sel",recPose,tempError,"No preceeding slp record")
      else:
        leftFlag                = False
        tempDict                = recPose.copy()
        widthBetweenStartAndEnd = getDistanceBetweenNodes(leftDictItem,tempDict) 
        if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
          # Invalid tape width
          processValueErrorHelper("slp",leftDictItem,tempError,
                                  "Invalid intersection width: {0}".format(widthBetweenStartAndEnd))
        else:
          # Records good, record it
          centerOfIntersection = utilities.getMidpointBetweenPoints(leftDictItem[0],leftDictItem[1],
                                                                    tempDict[0], tempDict[1])
          tempDict["x"] = centerOfIntersection[0]
          tempDict["y"] = centerOfIntersection[1]
          tempDict["<"] = utilities.getAngleAfterAdjustment(tempDict["<"],-90)  # Turn left 90'
          processValueHelper("sli",tempDict,tempList)

    # Start of right intersection
    if dictItem["srp"] == 1:
      if rightFlag: # We seeing a new start without closing old end
        processValueErrorHelper("srp",recPose,tempError,"Open srp record")
      else:
        rightFlag     = True
        rightDictItem = recPose.copy()

    # End of left intersection
    if dictItem["ser"] == 1:
      if rightFlag == False:  # didn't see start of intersection
        processValueErrorHelper("ser",recPose,tempError,"No preceeding srp record")
      else:
        rightFlag               = False
        tempDict                = recPose.copy()
        widthBetweenStartAndEnd = getDistanceBetweenNodes(rightDictItem,tempDict) 
        if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
          # Invalid tape width
          processValueErrorHelper("srp",rightDictItem,tempError,
                                  "Invalid intersection width: {0}".format(widthBetweenStartAndEnd))
        else:
          # Records good, record it
          centerOfIntersection = utilities.getMidpointBetweenPoints(rightDictItem[0],rightDictItem[1],
                                                                    tempDict[0], tempDict[1])
          tempDict["x"] = centerOfIntersection[0]
          tempDict["y"] = centerOfIntersection[1]
          tempDict["<"] = utilities.getAngleAfterAdjustment(tempDict["<"],90)  # Turn right 90'
          processValueHelper("sri",tempDict,tempList)

  # If error's we'll put the path we were just on back onto the list of paths to visit
  if len(tempError) > 0:   
    angle2Travel   = utilities.getAngleAfterAdjustment(firstPose["<"],180)      
    path2Travel    = { "x1" : firstPose["x1"], "y1" : firstPose["y1"], "<" : angle2Travel, 
                      "TYPE" : "CORRECTION", "INFO" : "Had errors on path" }
    gv.paths2Visit.append(path2Travel)
  else:
    # No errors process the data in tempList
    lastDict = {}
    for dictItem in tempList:
      gv.worldXMin = min(gv.worldXMin, dictItem["x"])
      gv.worldXMax = max(gv.worldXMax, dictItem["x"])
      gv.worldYMin = min(gv.worldYMin, dictItem["y"])
      gv.worldYMax = max(gv.worldYMax, dictItem["y"])

      currNodeId = getNodeAtPosition(dictItem["x"],dictItem["y"])
      # Add the node id to the dict item, better than repeating everywhere below, note tempList will reflect
      # this as dictItem is a reference variable
      dictItem["NODEID"] = currNodeId
      gv.pathsVisited.append(dictItem.copy())

      if dictItem["TYPE"] == "sas":  # At start of maze
        if len(gv.startPosition) == 0:
          gv.startNode = currNodeId
          gv.startDict = dictItem.copy()

      # At exit, add this to the list of potential goals
      if dictItem["TYPE"] == "sae":      
        gv.potentialGoalNodes.append(currNodeId)
        gv.potentialGoalDicts.append(dictItem.copy())
        nodeConnectionHelper(lastDict,dictItem)  # Add this node and the last one to the nodeConnectionList
        
      # If it is a goal then set the goal node identifier
      if dictItem["TYPE"] == "sgl":        
        gv.goalNode = currNodeId
        gv.goalDict = dictItem.copy()

      if dictItem["TYPE"] == "sli" or dictItem["TYPE"] == "sri":        
        nodeConnectionHelper(lastDict,dictItem)  
        gv.paths2Visit.append(dictItem.copy())  # Put this node as one to visit"
      
      # Save this item as the last one looked at if not sgl
      if dictItem["TYPE"] != "sgl":
        lastDict = dictItem.copy()

 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()

# --------------------------------------------------------------------------------------------------    
# This is called when we've read the 'start' 'end' block of data from the sparki
# The block of data sent represents a line segment, the values are stored in
# array 'pathValueList', each row in that is a dictionary item
# State legend: sas-EntranceOfMaze, sol-OnALine, sae-AtExit, slp-StartLeftPath,
#               srp-StartRightPath, sel-EndLeftPath, ser-EndRightPath, sgl-Goal
def OLDprocessValueList():
 try:
  initValues = True
  startX = 0.0
  startY = 0.0
  endX = 0.0
  endY = 0.0
  leftIntersections = []
  rightIntersections = []
  for dictItem in gv.pathValueList:
    recPose = { "x1" : dictItem["x"], "y1" : dictItem["y"], "<" : dictItem["<"] }
    # If need to initialize or "sas" (At start of maze) then set values
    if initValues == True or dictItem["sas"] == 1:
      segmentPose = recPose.copy()
      startX      = dictItem["x"]
      startY      = dictItem["y"]
      endX        = startX
      endY        = startY
      initValues  = False
    else:
      endX = dictItem["x"]
      endY = dictItem["y"]


    # If see 'At Start' of maze (and not seen before) set variable
    if dictItem["sas"] == 1:
      if len(gv.startPosition) == 0:
        gv.startPosition = recPose.copy()

    # At exit, add this to the list of potential goals
    if dictItem["sae"] == 1:
      gv.potentialGoals.append(recPose)

    # If it is a goal then set the goal position to this
    if dictItem["sgl"] == 1:
      gv.goalPosition = recPose.copy()

    # Start or end of the left path, put into array for later processing
    if dictItem["slp"] == 1 or dictItem["sel"] == 1:
      tempDict = recPose.copy()
      if dictItem["slp"] == 1:  # left path start
        tempDict["TYPE"] = "START"
      else:
        tempDict["TYPE"] = "END"
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
    gv.pathsVisited.append(segmentPose)
    # CHANGE ABOVE TO USE NODE DESIGNATION... 
  
    gv.worldXMin = min(gv.worldXMin, startX, endX)
    gv.worldXMax = max(gv.worldXMax, startX, endX)
    gv.worldYMin = min(gv.worldYMin, startY, endY)
    gv.worldYMax = max(gv.worldYMax, startY, endY)
   
    processIntersections("Left",-90,leftIntersections)
    processIntersections("Right",90,rightIntersections)

    if len(gv.errorList) > 0:  # Have errors we'll redo this line
      # We use the last position and change angle to 180' away from that
      angle2Travel   = utilities.getAngleAfterAdjustment(recPose["<"],180)      
      path2Travel    = { "x1" : recPose["x1"], "y1" : recPose["y1"], "<" : angle2Travel, 
                         "TYPE" : "CORRECTION", "INFO" : "Had errors on path" }
      gv.paths2Visit.append(path2Travel)

 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()

# --------------------------------------------------------------------------------------------------
# Gets the input line from the sparki, parses it into the respective dictionary
# object and puts it into the 'gv.pathValueList' array
def setPathValueListFromString(stringFromSparki):
  # Store values, we care about StateChg and POses, we'll make them
  #   dictionary objects and put them into the gv.pathValueList array
  arrayOfValues = stringFromSparki.split(",")
  if len(arrayOfValues) > 2:  # Has to has more than two values
    keyWord = arrayOfValues[0]
    if keyWord == "IR" and arrayOfValues[1].upper() == "STATECHG":
      if gv.DEBUGGING:
        print("in IR processing")
      stateObj = getState(arrayOfValues)
      gv.pathValueList.append(stateObj)
    
    if keyWord == "PO":  # This is still here but I moved the pose info into the IR record so 
      if gv.DEBUGGING:        #   it's probably not needed.... left in case :)
        print("In POSE processing")
      logPose = getPose(arrayOfValues)
      gv.pathValueList.append(logPose)
      if gv.DEBUGGING:
        print(logPose)
    if gv.DEBUGGING:
      print("Leaving setVars, len of array {0}".format(len(gv.pathValueList)))

def tellSparkiWhatToDo():
  return "Nothing"

def writeVariables():
  print("Start position: {0}".format(str(gv.startPosition)))
  print("World xMin: {0} yMin: {1} xMax: {2} yMax: {3}".format(gv.worldXMin,gv.worldYMin,gv.worldXMax,gv.worldYMax))
  
  print("PathsVisited:")
  for aPathVisited in gv.pathsVisited:
    print("  {0}".format(str(aPathVisited)))
  
  print("Paths2Visit:")
  for aPath2Visit in gv.paths2Visit:
    print("  {0}".format(str(aPath2Visit)))
  
  if len(gv.goalPosition) > 0:
    print("Goal position {0}".format(str(gv.goalPosition)))
  else:
    print("Goal not found")

  if len(gv.potentialGoals) > 0:
    print("Potential goals:")
    for aPotentialGoal in gv.potentialGoals:
      print("  {0}".format(str(aPotentialGoal)))
  else:
    print("No Potential goals found")
    
  