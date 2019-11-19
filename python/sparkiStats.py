
from statistics import mean, median, mode, stdev
import os
import sys

import sharedVars as gv
import utilities
import dijkstrasClass

SENSORDISTANCEPASTTAPE = 0.2

"""
To do:
    Check that when pass intersection it doesn't register sae unless it's online again, and may
    want to have some min distance
      DONE
        For instructions
          get the graph of nodeconnectionlist graph<-getGraphOfNodeConnectionList()
          get the current node gv.closestNodePose["NODEID"]
          call dijkstra with currentNode and graph, it'll return array with costs
            to all the other nodes
          minCost = 99999999
          savePosition = -1
          for path2Visit in gv.paths2Visit:
            if costTo_Path2Visit < minCost
              savePosition (idx)
              minCost <- cosTo_Path2Visit
          if savePosition != -1
            Remove element at savePosition
            tell Sparki to go to that position (dijkstra returns the path to it too)

  Display a matplot of the line segments 

  Have 'mode' which is 'EXPLORE', 'GOAL'
"""  


"""
Sample reading (put in breaks to make readable):

IR,StateChg,x,12.09,y,0.00,<,0,el,980,ll,973,lc,955,lr,915,er,980,
  ell,0,lll,0,lcl,0,lrl,0,erl,0,
  sdl,0,sdr,0,sol,0,sae,1,slp,0,srp,0,sel,0,ser,0,sas,0,sgl,0


nodeList = [] # List of nodes
nodeConnectionList = [] # Nodes and node there connected, with angle and distance between them
nodesToVisit = [] # Nodes to visit and the angle they should visit
"""

# --------------------------------------------------------------------------------------------------
# This handles passing instructions to caller... what we do
#   for all nodes in nodes2Target (except for the last node) we'll have 'GOTO' instructions
#   for the last value we use the 'finalPath' (it has <) and we'll prefix the last one
#     with the pathType (a string that tells what this last type is)
def getDirectionsForNodes(nodes2Target, finalPath, pathType):
  if len(nodes2Target) > 1:
    rtnString = "GOTO"
    for nodeIdx in range(len(nodes2Target)-1):
      node2GoTo = nodes2Target[nodeIdx]
      rtnString += ",x," + str(gv.nodeList[node2GoTo]["x"]) + ",y," + str(gv.nodeList[node2GoTo]["y"])
    rtnString += "," + pathType
  else:
    rtnString = pathType

  # Now we add the last item, it has the orientation the robot should be at for the 'pathType'
  rtnString += ",x," + str(finalPath["x"]) + ",y," + str(finalPath["y"]) + ",<," + str(finalPath["<"])
  return rtnString

# --------------------------------------------------------------------------------------------------
# Little helper to return distance between two node objects
def getDistanceBetweenNodes(node1, node2):
  return utilities.getDistanceBetweenPoints(node1["x"], node1["y"], node2["x"], node2["y"])

# --------------------------------------------------------------------------------------------------
# Return a graph (dictionary) of each node and it's neighbors (also a dict), so it'll be
# of form { nodeId : {adjacentNode1 : distance, adjacentNode2 : distance}, nodeId2 : {adjacentNode1 : distance, ...}}
def getGraphOfNodeConnectionList():
  graphToReturn = {}
  for nodeDict in gv.nodeConnectionList:
    nodeId1  = nodeDict["id1"]
    neighbor = nodeDict["id2"]
    if nodeId1 not in graphToReturn:
      graphToReturn[nodeId1] = {}
    if neighbor not in graphToReturn[nodeId1]:
      graphToReturn[nodeId1][neighbor] = nodeDict["len"]
  return graphToReturn.copy()

# --------------------------------------------------------------------------------------------------
# Return the node that is at this position (or within TAPEWIDTH away from it), it not found
# then a new node will be created (if last arg is true)
def getNodeAtPosition(x, y, createIfNonExistant=True):
  nodeIdToReturn = -1
  for idx in range(len(gv.nodeList)):
    if utilities.getDistanceBetweenPoints(x,y,gv.nodeList[idx]["x"],gv.nodeList[idx]["y"]) <= gv.TAPEWIDTH:
      nodeIdToReturn = idx
  if nodeIdToReturn == -1 and createIfNonExistant: # Not found, add it
    nodeIdToReturn = len(gv.nodeList) # Len is 1 greater than index position, so can use it without any modification
    gv.nodeList.append({"x" : x, "y" : y})
  return nodeIdToReturn

# --------------------------------------------------------------------------------------------------
# See if the node passed in is already the list... if so return it's position in the list
# othewise we'll return -1
def getNodeConnectionPositionInList(node2Check):
  thePosition = -1
  for tempIdx in range(len(gv.nodeConnectionList)):
    if ((gv.nodeConnectionList[tempIdx]["id1"] == node2Check["id1"]) and 
        (gv.nodeConnectionList[tempIdx]["id2"] == node2Check["id2"])):
      thePosition = tempIdx
  return thePosition

# --------------------------------------------------------------------------------------------------
# Get the next goal to visit, return a the list of nodes to get there, and the final item pose
def getNextGoal2Visit():
 try:
  currentNodeId = gv.closestNodePose["NODEID"]
  currentGraph  = getGraphOfNodeConnectionList()
  dijkstrasObj  = dijkstrasClass.dijkstraGraphPoints(currentGraph, currentNodeId)

  # If there is a goal node and we didn't visit it then return that
  if ((len(gv.goalDict) > 0) and (gv.goalDict["NODEID"] not in gv.goalsVisited)):
    gv.goalsVisited.append(gv.goalDict["NODEID"])
    thePath2Goal = dijkstrasObj.getShortestPathToGoal(gv.goalDict["NODEID"])
    return thePath2Goal, gv.goalDict.copy()
  else:
    minPos  = -1
    minCost = 9999999
    for gidx in range(len(gv.potentialGoalDicts)):
      theDictItem = gv.potentialGoalDicts[gidx]
      if (theDictItem["NODEID"] not in gv.goalsVisited):
        cost2GetThere = dijkstrasObj.getDistanceToNode(theDictItem["NODEID"])
        if cost2GetThere < minCost:
          minCost = cost2GetThere
          minPos  = gidx
    if minPos >= 0:
      theDictItem = gv.potentialGoalDicts[minPos].copy()
      gv.goalsVisited.append(theDictItem["NODEID"])
      thePath2Goal = dijkstrasObj.getShortestPathToGoal(theDictItem["NODEID"])
      return thePath2Goal, theDictItem
    else:
      return [], {}
 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()

# --------------------------------------------------------------------------------------------------
# Return a list that has the path from the current position to the next path to visit and
# also the dictionary item for the final location
def getNextPath2Visit():
  try:
    currentNodeId = gv.closestNodePose["NODEID"]
    currentGraph  = getGraphOfNodeConnectionList()
    dijkstrasObj  = dijkstrasClass.dijkstraGraphPoints(currentGraph, currentNodeId)
    minCost       = 9999999
    pidxWithMin   = -1
    for pidx in range(len(gv.paths2Visit)):
      path2Check    = gv.paths2Visit[pidx]
      cost2GetThere = dijkstrasObj.getDistanceToNode(path2Check["NODEID"])
      if cost2GetThere < minCost:
        minCost     = cost2GetThere
        pidxWithMin = pidx
    if pidxWithMin >= 0:
      thePath2Visit = gv.paths2Visit[pidxWithMin]  # Next path to visit
      gv.paths2Visit.pop(pidxWithMin)              # Remove it from the list
      thePath2Goal = dijkstrasObj.getShortestPathToGoal(thePath2Visit["NODEID"])
      return thePath2Goal, thePath2Visit
    else:
      return [], {}
  except:
    return [], {}

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
 try:
  idx = 2 # Have it represent where the value is not the label for it
  stateToReturn = { "RECTYPE" : "STATE" }
  while idx < len(arrayValues):
    # print(str(arrayValues[idx]))
    sensorLabel = arrayValues[idx]
    if sensorLabel == "x" or sensorLabel == "y":
      sensorValue = float(arrayValues[idx+1])
    else:
      sensorValue = int(arrayValues[idx+1])
    idx += 2
    stateToReturn[sensorLabel] = sensorValue
  return stateToReturn
 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()
# --------------------------------------------------------------------------------------------------
# Little helper routine to see if the dictionary item passed in (with NODEID, <) 
# already matches a path that has been visited before.
def hasPathBeenVisited(dictOfPath2Check):
  beenVisited = False
  for nodeAlreadyVisited in gv.pathsVisited:
    if dictOfPath2Check["NODEID"] == nodeAlreadyVisited["NODEID"]:
      if utilities.areAnglesCloseEnough(dictOfPath2Check["<"],nodeAlreadyVisited["<"]):
        beenVisited = True
  return beenVisited

# --------------------------------------------------------------------------------------------------
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

  # See if it exists... if position returned is -1 it doesn't so add it.
  if getNodeConnectionPositionInList(nodeConnection) == -1:                     
    gv.nodeConnectionList.append(nodeConnection.copy())
  
  # We want to store the node connection from id2 back to id1 also
  nodeConnection["id1"] = nodeConnection["id2"]
  nodeConnection["id2"] = lastDict["NODEID"]
  nodeConnection["<"]   = utilities.getAngleAfterAdjustment(nodeConnection["<"],180)

  if getNodeConnectionPositionInList(nodeConnection) == -1:                     
    gv.nodeConnectionList.append(nodeConnection.copy())

# --------------------------------------------------------------------------------------------------
# Helper to add a dictionary item to a list; we populate the TYPE of record based
# on the argument passed in
def processValueHelper(typeRecord, dictItem, list2Add2):
  tempPose         = dictItem.copy()
  tempPose["TYPE"] = typeRecord
  list2Add2.append(tempPose)

# --------------------------------------------------------------------------------------------------
# Little helper routine to add record passed in into the errorList (also passed in)
def processValueErrorHelper(typeRecord, dictItem, errorList2Add2, errMessage):
  tempPose          = dictItem.copy()
  tempPose["TYPE"]  = typeRecord
  tempPose["ERROR"] = errMessage
  errorList2Add2.append(tempPose)

# --------------------------------------------------------------------------------------------------    
# This is called when we've read the 'start' 'end' block of data from the sparki
# The block of data sent represents a line segment, the values are stored in
# array 'pathValueList', each row in that is a dictionary item
# State legend: sas-EntranceOfMaze, sol-OnALine, sae-AtExit, slp-StartLeftPath,
#               srp-StartRightPath, sel-EndLeftPath, ser-EndRightPath, sgl-Goal
def processValueList():
 try:
  initValues = True
  tempList   = []
  # tempError  = []
  leftFlag   = False
  rightFlag  = False
  
  # We do this in two passes, during the first pass we prep the tempList array with formatted
  # data; we also do validation during this phase.  If there are no errors then we'll process
  # the tempList array and update the global variables.
  for dictItem in gv.pathValueList:
    recPose = { "x" : dictItem["x"], "y" : dictItem["y"], "<" : dictItem["<"] }
    gv.finalPose = recPose.copy()
    
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
        processValueErrorHelper("slp",recPose,gv.errorList,"Open slp record")
      else:
        leftFlag     = True
        leftDictItem = recPose.copy()

    # End of left intersection  NOTE the ending position should probably be adjusted by SENSORDISTANCEPASTTAPE
    # since it's past the tape when sensor goes off... do the same for ser
    if dictItem["sel"] == 1:
      if leftFlag == False:  # didn't see start of intersection
        processValueErrorHelper("sel",recPose,gv.errorList,"No preceeding slp record")
      else:
        leftFlag                = False
        tempDict                = recPose.copy()
        widthBetweenStartAndEnd = getDistanceBetweenNodes(leftDictItem,tempDict) 
        if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
          # Invalid tape width
          processValueErrorHelper("slp",leftDictItem,gv.errorList,
                                  "Invalid intersection width: {0}".format(widthBetweenStartAndEnd))
        else:
          # Records good, record it
          centerOfIntersection = utilities.getMidpointBetweenPoints(leftDictItem["x"],leftDictItem["y"],
                                                                    tempDict["x"], tempDict["y"])
          tempDict["x"] = centerOfIntersection[0]
          tempDict["y"] = centerOfIntersection[1]
          tempDict["dest<"] = utilities.getAngleAfterAdjustment(tempDict["<"],-90)  # Turn left 90'
          processValueHelper("sli",tempDict,tempList)

    # Start of right intersection
    if dictItem["srp"] == 1:
      if rightFlag: # We seeing a new start without closing old end
        processValueErrorHelper("srp",recPose,gv.errorList,"Open srp record")
      else:
        rightFlag     = True
        rightDictItem = recPose.copy()

    # End of left intersection
    if dictItem["ser"] == 1:
      if rightFlag == False:  # didn't see start of intersection
        processValueErrorHelper("ser",recPose,gv.errorList,"No preceeding srp record")
      else:
        rightFlag               = False
        tempDict                = recPose.copy()
        widthBetweenStartAndEnd = getDistanceBetweenNodes(rightDictItem,tempDict) 
        if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
          # Invalid tape width
          processValueErrorHelper("srp",rightDictItem,gv.errorList,
                                  "Invalid intersection width: {0}".format(widthBetweenStartAndEnd))
        else:
          # Records good, record it
          centerOfIntersection = utilities.getMidpointBetweenPoints(rightDictItem["x"],rightDictItem["y"],
                                                                    tempDict["x"], tempDict["y"])
          tempDict["x"] = centerOfIntersection[0]
          tempDict["y"] = centerOfIntersection[1]
          tempDict["dest<"] = utilities.getAngleAfterAdjustment(tempDict["<"],90)  # Turn right 90'
          processValueHelper("sri",tempDict,tempList)

  # If error's we'll put the path we were just on back onto the list of paths to visit
  if len(gv.errorList) > 0:   
    angle2Travel   = utilities.getAngleAfterAdjustment(firstPose["<"],180)      
    path2Travel    = { "x" : firstPose["x"], "y" : firstPose["y"], "<" : angle2Travel, 
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
          gv.startDict = dictItem.copy()

      # At exit, add this to the list of potential goals
      if dictItem["TYPE"] == "sae":      
        gv.potentialGoalNodes.append(currNodeId)
        gv.potentialGoalDicts.append(dictItem.copy())
        nodeConnectionHelper(lastDict,dictItem)  # Add this node and the last one to the nodeConnectionList
        
      # If it is a goal then set the goal node identifier
      if dictItem["TYPE"] == "sgl":        
        gv.goalDict = dictItem.copy()

      if dictItem["TYPE"] == "sli" or dictItem["TYPE"] == "sri":   
        if (gv.DEBUGGING):
          print("calling nodeConnectionHelper: ")
          print(str(lastDict))
          print(str(dictItem))     
        nodeConnectionHelper(lastDict,dictItem)  
        intersectDict = dictItem.copy()
        # Want to put the destination angle into the < element, we'll
        # save orig< just in case :)
        intersectDict["orig<"] = intersectDict["<"]
        intersectDict["<"]     = intersectDict["dest<"]
        # Check that it hasn't already been visited, if not add it to the list
        if hasPathBeenVisited(intersectDict)  == False:
          gv.paths2Visit.append(intersectDict.copy()) 
        # Don't know why I had this here... it was making the current pose the wrong angle... I changed
        # by commenting out this and using dictItem.copy()... if after testing you find no issues then
        # change this so that it's only done in one spot.. not here and then in the else
        # gv.closestNodePose = intersectDict.copy()
        gv.closestNodePose = dictItem.copy()
      else:
        gv.closestNodePose = dictItem.copy()  # Save the current pose (the last one seen)

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
# Gets the input line from the sparki, parses it into the respective dictionary
# object and puts it into the 'gv.pathValueList' array
def setPathValueListFromString(stringFromSparki):
 try:
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
 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()
# --------------------------------------------------------------------------------------------------
# Sparki wants instructions; if there's paths to visit then give him the best one
# if there aren't any paths then start checking goals
def tellSparkiWhatToDo():
 try:
  movementType = 'XPLORE'
  nodes2Target, gv.pathBeingVisited = getNextPath2Visit()
  if len(gv.pathBeingVisited) == 0:  # No paths left to visit see if there's a goal
    movementType = 'GOAL'
    nodes2Target, gv.pathBeingVisited = getNextGoal2Visit()

  if len(gv.pathBeingVisited) == 0: # No nodes or goals to vist tell em we're done
    return "Done"
  else:
    return getDirectionsForNodes(nodes2Target, gv.pathBeingVisited, movementType)
 except:
  print("Exception raised in serialProcessor.py")
  exc_type, exc_obj, exc_tb = sys.exc_info()
  fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
  print(exc_type, fname, exc_tb.tb_lineno)
  print("Exception raised flushing serial port")  
  sys.exit()
# --------------------------------------------------------------------------------------------------
# Write all the variables of interest out to the console
def writeVariables():
  print("Start position: {0}".format(str(gv.startDict)))
  print("Current position: {0}".format(str(gv.finalPose)))
  print("Closes node position: {0}".format(str(gv.closestNodePose)))
  print("World xMin: {0} yMin: {1} xMax: {2} yMax: {3}".format(gv.worldXMin,gv.worldYMin,gv.worldXMax,gv.worldYMax))
  
  print("PathsVisited:")
  for aPathVisited in gv.pathsVisited:
    print("  {0}".format(str(aPathVisited)))
  
  print("Paths2Visit:")
  for aPath2Visit in gv.paths2Visit:
    print("  {0}".format(str(aPath2Visit)))
  
  print("pathBeingVisited")
  print("  {0}".format(str(gv.pathBeingVisited)))

  if len(gv.goalDict) > 0:
    print("Goal position {0}".format(str(gv.goalDict)))
  else:
    print("Goal not found")

  if len(gv.potentialGoalDicts) > 0:
    print("Potential goals:")
    for aPotentialGoal in gv.potentialGoalDicts:
      print("  {0}".format(str(aPotentialGoal)))
  else:
    print("No Potential goals found")

  if len(gv.nodeList) > 0:
    print("Node list below")
    for nodeId in range(len(gv.nodeList)):
      print("NodeId: {0} value: {1}".format(nodeId,str(gv.nodeList[nodeId])))
  
  if len(gv.nodeConnectionList) > 0:
    print("Node connection list below")
    for aNodeConnection in gv.nodeConnectionList:
      print(str(aNodeConnection))
    print("Node connection as a graph")
    print("  " + str(getGraphOfNodeConnectionList()))

  if len(gv.nodesToVisit) > 0:
    print("Nodes to visit")
    for aNode in gv.nodesToVisit:
      print(str(aNode))
