
from statistics import mean, median, mode, stdev
import os
import sys
import pickle

import sharedVars as gv
import utilities
import dijkstrasClass

SENSORDISTANCEPASTTAPE = 0.2

"""
C_EXPLORE = "X"
C_GOAL    = "G"
C_DONE    = "Q"
C_GOTO    = "M"

To do:
    Check that when pass intersection it doesn't register sae unless it's online again, and may
    want to have some min distance
      DONE
        For instructions
          get the graph of nodeconnectionlist graph<-getGraphOfNodeConnectionList()
          get the current node gv.lastRobotDictItem["NODEID"]
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

# Helper method, pass in the indicator as to whether we saw an obstacle (called after checking a goal)
# if the flag is on then we'll set the global (goalFound) to be the goal that was being checked (goalBeingChecked)
# We'll also set goalDict to have it's position and save the result to the pickle file for future runs
def checkAndSetGoalStatus(sawObstacle):
  if ((gv.goalBeingChecked != ' ') and (sawObstacle == True)):
    gv.goalFound   = gv.goalBeingChecked
    posInGoalIndex = getPotentialGoalNodeIndex(gv.goalFound)
    gv.goalDict    = gv.potentialGoalDicts[posInGoalIndex]
    zSaveGoalElementToPickle()

# -------------------------------------------------------------------------
# Takes a sensor pose and returns the pose of the center of the robot.  The
# last arg is defaulted at True as the center of the robot is behind the
# sensor... the only time this may not be the case is when you want to log
# the pose when you first enter the maze in which case you may want 4cm
# in front of the sensor 
# NOTE: you can also use this to take a robot pose and return the sensor
#       pose.. that's also good use of the second argument... so if you
#       give it the robot pose, and False (arg) it'll give u the sensor pose
def getActualPoseOfSensorPose(sensorPose, poseBehindSensor=True):
  try:
    tempDictItem = sensorPose.copy() # Make copy, this is the item we'll return
    if "<" in tempDictItem:
      theGoalAngle = tempDictItem["<"]  
      if poseBehindSensor:
        # Subtract the distance that the sensor is ahead of the body of the robot to get the new x and y position
        tempDictItem["x"] = tempDictItem["x"] - (gv.INFRARED_SENSOR_FORWARD_OF_CENTER * utilities.degreesCos(theGoalAngle))
        tempDictItem["y"] = tempDictItem["y"] - (gv.INFRARED_SENSOR_FORWARD_OF_CENTER * utilities.degreesSin(theGoalAngle))
      else:
        tempDictItem["x"] = tempDictItem["x"] + (gv.INFRARED_SENSOR_FORWARD_OF_CENTER * utilities.degreesCos(theGoalAngle))
        tempDictItem["y"] = tempDictItem["y"] + (gv.INFRARED_SENSOR_FORWARD_OF_CENTER * utilities.degreesSin(theGoalAngle))
    else:
      raise NameError("getActualPoseOfSensorPose(No < in {0} )".format(str(tempDictItem)))

    tempDictItem["x"] = round(tempDictItem["x"],2)
    tempDictItem["y"] = round(tempDictItem["y"],2)
    return tempDictItem
  except:
    print("Exception raised - getDirectionsForNodes")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()  


# --------------------------------------------------------------------------------------------------
# This handles passing instructions to caller... what we do
#   for all nodes in nodes2Target (except for the last node) we'll have 'GOTO' instructions
#   for the last value we use the 'finalPath' (it has <) and we'll prefix the last one
#     with the pathType (a string that tells what this last type is)

# Changed to include all nodes in nodes2Target... that way the 'goto' is the goto the
# node, and the pathType verb is really just handling the angle change 

def getPotentialGoalNodeIndex(theNodeId):
  rtnValue = -1
  for tempGidx in range(len(gv.potentialGoalDicts)):
    if gv.potentialGoalDicts[tempGidx]["NODEID"] == theNodeId:
      rtnValue = tempGidx
  return rtnValue


# Returns a string that has the directions to go to the 'finalPath', the args are
#  nodes2Target - This is an array of the nodes that we need to go thru to get to the final path
#  finalPath - a dictionary item that has the final path and angle that we want to be at
def getDirectionsForNodes(nodes2Target, finalPath, pathType):
  try:
    if (gv.DEBUGGING == False): writeHelper("in getDirectionsForNodes")
    if len(nodes2Target) > 1:
      rtnString = ""
      for nodeIdx in range(len(nodes2Target)):
        node2GoTo = nodes2Target[nodeIdx]
        workNode  = gv.nodeList[node2GoTo]  # The index in nodeList is the node id
        if (gv.DEBUGGING == False): writeHelper("  node2GoTo: {0} workNode: {1}".format(node2GoTo,str(workNode)))
        rtnString += gv.C_GOTO + ",x," + str(workNode["x"]) + ",y," + str(workNode["y"]) + ",<,-1,"
      rtnString += pathType
    else:
      if len(gv.robotPoseFromInstruction) > 0:   # $$$$$$$$$ CHECK WHY NEED THIS
        # Have a goto so that we position from current robot pose to the 
        rtnString = gv.C_GOTO + ",x," + str(finalPath["x"]) + ",y," + str(finalPath["y"]) + ",<,-1," + pathType
      else:
        rtnString = pathType
    
    # Now we add the last item, it has the orientation the robot should be at for the 'pathType'
    rtnString += ",x," + str(finalPath["x"]) + ",y," + str(finalPath["y"]) + ",<," + str(finalPath["<"])
    return rtnString
  except:
    print("Exception raised - getDirectionsForNodes")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()  

# --------------------------------------------------------------------------------------------------
# Little helper to return distance between two node objects
def getDistanceBetweenNodes(node1, node2):
  return utilities.getDistanceBetweenPoints(node1["x"], node1["y"], node2["x"], node2["y"])

# --------------------------------------------------------------------------------------------------
# Return a graph (dictionary) of each node and it's neighbors (also a dict), so it'll be
# of form { nodeId : {adjacentNode1 : distance, adjacentNode2 : distance}, nodeId2 : {adjacentNode1 : distance, ...}}
def getGraphOfNodeConnectionList():
  try:
    graphToReturn = {}
    for nodeDict in gv.nodeConnectionList:
      nodeId1  = nodeDict["id1"]
      neighbor = nodeDict["id2"]
      if nodeId1 not in graphToReturn:
        graphToReturn[nodeId1] = {}
      if neighbor not in graphToReturn[nodeId1]:
        graphToReturn[nodeId1][neighbor] = nodeDict["len"]
    if (gv.DEBUGGING): 
      writeHelper("getGraphOfNodeConnectionList, graph is:")
      writeHelper("  " + str(graphToReturn))
    return graphToReturn.copy()
  except:
    print("Exception raised - getGraphOfNodeConnectionList")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()  

# --------------------------------------------------------------------------------------------------
# Return the node that is at this position (or within TAPEWIDTH away from it), it not found
# then a new node will be created (if last arg is true)
def getNodeAtPosition(x, y, createIfNonExistant=True):
  try:
    nodeIdToReturn = -1
    for idx in range(len(gv.nodeList)):
      # Was using distance of gv.TAPEWIDTH/2 but changed to tapwidth
      if utilities.getDistanceBetweenPoints(x,y,gv.nodeList[idx]["x"],gv.nodeList[idx]["y"]) <= (gv.TAPEWIDTH):
        nodeIdToReturn = idx
    if nodeIdToReturn == -1 and createIfNonExistant: # Not found, add it
      nodeIdToReturn = len(gv.nodeList) # Len is 1 greater than index position, so can use it without any modification
      gv.nodeList.append({"x" : x, "y" : y, "NODEID" : nodeIdToReturn}) # Added nodeid 11/23 so can use in getDirection...
      writeSpeech("Added node {0}".format(nodeIdToReturn))
    return nodeIdToReturn
  except:
    print("Exception raised - getNodeAtPosition")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()  

# --------------------------------------------------------------------------------------------------
# See if the node passed in is already the list... if so return it's position in the list
# othewise we'll return -1
def getNodeConnectionPositionInList(node2Check):
  try:
    thePosition = -1
    for tempIdx in range(len(gv.nodeConnectionList)):
      if ((gv.nodeConnectionList[tempIdx]["id1"] == node2Check["id1"]) and 
          (gv.nodeConnectionList[tempIdx]["id2"] == node2Check["id2"])):
        thePosition = tempIdx
    return thePosition
  except:
    print("Exception raised - getNodeConnectionPositionInList")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()  

# --------------------------------------------------------------------------------------------------
# Get the next goal to visit, return a the list of nodes to get there, and the final item pose
def getNextGoal2Visit():
  try:
    if "NODEID" in gv.lastRobotDictItem:
      currentNodeId = gv.lastRobotDictItem["NODEID"]
    else:
      currentNodeId = ''
    print("in getNextGoal2Visit, currentNodeId {0}".format(currentNodeId))
    currentGraph = getGraphOfNodeConnectionList()
    dijkstrasObj = dijkstrasClass.dijkstraGraphPoints(currentGraph, currentNodeId)
    gv.goalBeingChecked = ' ' # Clear existing value if set

    # If there is a goal node and we didn't visit it then return that
    if ((len(gv.goalDict) > 0) and (gv.goalDict["NODEID"] not in gv.goalsVisited)):
      gv.goalsVisited.append(gv.goalDict["NODEID"])
      gv.goalBeingChecked = gv.goalDict["NODEID"]
      thePath2Goal = dijkstrasObj.getShortestPathToGoal(gv.goalDict["NODEID"])
      print("in getNextGoal2Visit, gv.goalDict logic, path: {0}, dict: {1}".format(str(thePath2Goal),gv.goalDict))
      return thePath2Goal, gv.goalDict.copy()
    else:
      minPos  = -1
      minCost = 9999999
      for gidx in range(len(gv.potentialGoalDicts)):
        theDictItem = gv.potentialGoalDicts[gidx]
        if (theDictItem["NODEID"] not in gv.goalsVisited):
          cost2GetThere = dijkstrasObj.getDistanceToNode(theDictItem["NODEID"])
          if True:
            print("getNextGoal2Visit, From: {0} to {1} costs:{2}".format(currentNodeId,theDictItem["NODEID"],cost2GetThere))
          if cost2GetThere < minCost:
            minCost = cost2GetThere
            minPos  = gidx
      if minPos >= 0:
        theDictItem = gv.potentialGoalDicts[minPos].copy()
        gv.goalsVisited.append(theDictItem["NODEID"])
        gv.goalBeingChecked = theDictItem["NODEID"]
        thePath2Goal = dijkstrasObj.getShortestPathToGoal(theDictItem["NODEID"])
        print("in getNextGoal2Visit, node: {0}, path: {1}".format(theDictItem["NODEID"],str(thePath2Goal)))
  
        return thePath2Goal, theDictItem
      else:
        return [], {}
  except:
    print("Exception raised - getNextGoal2Visit")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Return a list that has the path from the current position to the next path to visit and
# also the dictionary item for the final location
# NOTE, since we may have visited a path while exploring 180' from it we'll first check
# if any of paths to visit were visited due to that...
def getNextPath2Visit():
  try:
    if (gv.DEBUGGING):  writeHelper("getNextPath2Vistit (gNP2V)")
      
    if "NODEID" in gv.lastRobotDictItem:
      currentNodeId = gv.lastRobotDictItem["NODEID"]
    else:
      currentNodeId = ''
    currentGraph  = getGraphOfNodeConnectionList()
    dijkstrasObj  = dijkstrasClass.dijkstraGraphPoints(currentGraph, currentNodeId)

    # We may have visited the path from the other direction, if so remove it from the list
    pidx = len(gv.paths2Visit) - 1
    while pidx >= 0:      
      path2Check = gv.paths2Visit[pidx].copy()
      #path2Check["<"] = utilities.getAngleAfterAdjustment(path2Check["<"],180)
      if hasPathBeenVisited(path2Check):
        print("Visited {0} from 180', we removed it".format(gv.paths2Visit[pidx]))
        gv.paths2Visit.pop(pidx)  # Remove it from the list we visited from other direction
      pidx -= 1

    # Ok we've prunned th list, get the path :)
    minCost       = 9999999
    pidxWithMin   = -1
    for pidx in range(len(gv.paths2Visit)):
      path2Check = gv.paths2Visit[pidx]
      isCorrection = False
      if "TYPE" in path2Check:
        isCorrection = (path2Check["TYPE"] == "CORRECTION")
      
      # If this path is on list cause it needs to be 'corrected' (i.e. error 
      # occurred on prior exploration) then this is most important one to 
      # visit... we need node information
      if (isCorrection == True):
        cost2GetThere = -1
      else:
        cost2GetThere = dijkstrasObj.getDistanceToNode(path2Check["NODEID"])
      if (gv.DEBUGGING): writeHelper("  (gNP2V) cost2GetThere: " + str(cost2GetThere))
      if cost2GetThere < minCost:
        minCost     = cost2GetThere
        pidxWithMin = pidx

    if pidxWithMin >= 0:
      thePath2Visit = gv.paths2Visit[pidxWithMin]  # Next path to visit
      gv.paths2Visit.pop(pidxWithMin)              # Remove it from the list
      if "NODEID" in thePath2Visit:
        thePath2Goal = dijkstrasObj.getShortestPathToGoal(thePath2Visit["NODEID"])
      else:
        thePath2Goal = []
      if (gv.DEBUGGING): writeHelper("  (gNP2V) thePath2Visit: " + str(thePath2Visit))
      return thePath2Goal, thePath2Visit
    else:
      if (gv.DEBUGGING): writeHelper("  (gNP2V) pidxWithMin < 0, no path")
      return [], {}
  except:
    print("Exception raised - getNextPath2Visit")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Return a dictionary for the pose, NOTE we convert the string numbers to correct representation in
# here.
def getPose(arrayValues):
  try:
    if gv.DEBUGGING:
      writeHelper("in getPose, len of args: {0}".format(len(arrayValues)))
    poseToReturn = { "RECTYPE" : "POSE" }
    idx = 1
    while (idx < len(arrayValues)):
      if gv.DEBUGGING:
        writeHelper("getPose key: {0} value:{1}:".format(arrayValues[idx],arrayValues[idx+1]))
      if arrayValues[idx] == "x" or arrayValues[idx] == "y":
        theValu = float(arrayValues[idx+1])
      else:
        theValu = int(arrayValues[idx+1])
      poseToReturn[arrayValues[idx]] = theValu
      if gv.DEBUGGING:
        writeHelper(poseToReturn)
      idx += 2
    # Convert sensor pose to robot pose
    poseToReturn = getActualPoseOfSensorPose(poseToReturn)
    if gv.DEBUGGING:
      writeHelper("poseToReturn: ")
      writeHelper(poseToReturn)
    return poseToReturn
  except:
    print("Exception raised - getPose")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

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
    print("Exception raised - getState")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Little helper routine to see if the dictionary item passed in (with NODEID, <) 
# already matches a path that has been visited before.
# 12/08/2019 added logic to check if it's connected to another node on that angle 
def hasPathBeenVisited(dictOfPath2Check):
  try:
    beenVisited = False
    for nodeAlreadyVisited in gv.pathsVisited:
      if dictOfPath2Check["NODEID"] == nodeAlreadyVisited["NODEID"]:
        if utilities.areAnglesCloseEnough(dictOfPath2Check["<"],nodeAlreadyVisited["<"]):
          print("xxx {0} was visited {1}".format(str(dictOfPath2Check),str(nodeAlreadyVisited)))
          beenVisited = True
    if (beenVisited == False): # See if it's already connected on the angle to another node
      for nodeAlreadyVisited in gv.nodeConnectionList:
        if dictOfPath2Check["NODEID"] == nodeAlreadyVisited["id1"]:
          if utilities.areAnglesCloseEnough(dictOfPath2Check["<"],nodeAlreadyVisited["<"]):
            print("xxxNode {0} was visited {1}".format(str(dictOfPath2Check),str(nodeAlreadyVisited)))
            beenVisited = True
    return beenVisited
  except:
    print("Exception raised - hasPathBeenVisited")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Little helper routine, this adds a dictionary item to the nodeConnectionList, it's
# in format { "id1" : nodeId1, "<" : angleFromID1toID2, "id2" :nodeId2, "len" : distanceBetweenThem}
# By default we won't add a node that is pointing to itself, not sure if down the road
# you'd want to allow (that's why made it an argument)
def nodeConnectionHelper(lastDict,currDict,point2Self=False):
  try:
    if (gv.DEBUGGING):
      writeHelper("nodeConnectionHelper, last: {0} curr: {1}".format(str(lastDict),str(currDict)))
    if ((lastDict["NODEID"] != currDict["NODEID"]) or (point2Self == True)):
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
  except:
    print("Exception - nodeConnectionHelper, lastDict: {0} currDict: {1}".format(str(lastDict),str(currDict)))
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Helper to add a dictionary item to a list; we populate the TYPE of record based
# on the argument passed in
def processValueHelper(typeRecord, dictItem, list2Add2):
  try:
    tempPose         = dictItem.copy()
    tempPose["TYPE"] = typeRecord
    list2Add2.append(tempPose)
  except:
    print("Exception raised - processValueHelper")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Little helper routine to add record passed in into the errorList (also passed in)
def processValueErrorHelper(typeRecord, dictItem, errorList2Add2, errMessage):
  try:
    tempPose          = dictItem.copy()
    tempPose["TYPE"]  = typeRecord
    tempPose["ERROR"] = errMessage
    errorList2Add2.append(tempPose)
  except:
    print("Exception raised - processValueErrorHelper")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()


# $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
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
    print(" ")
    print("IN PROCESSVALUELIST............. LEN: {0}".format(len(gv.pathValueList)))
    print(" ")
    # We do this in two passes, during the first pass we prep the tempList array with formatted
    # data; we also do validation during this phase.  If there are no errors then we'll process
    # the tempList array and update the global variables.
    # NOTE: the poses in the first pass are the sensor poses
    for dictItem in gv.pathValueList:
      sensorPose = { "x" : dictItem["x"], "y" : dictItem["y"], "<" : dictItem["<"] }
      robotPose = getActualPoseOfSensorPose(sensorPose)
      
      if initValues: # Save first record just in case we have to do a doover
        firstRobotPose = robotPose.copy()

      # If it's a pose record skip it... we only have it for setting position
      if dictItem["RECTYPE"] == "POSE":
        continue 

      if dictItem["sas"] == 1:
        # This 'getRobotPose' is in front of the sensor that's why a diff call
        processValueHelper("sas",getActualPoseOfSensorPose(robotPose,False),tempList)
      
      if dictItem["sae"] == 1:
        processValueHelper("sae",robotPose,tempList)
      
      if dictItem["sgl"] == 1:
        processValueHelper("sgl",robotPose,tempList)
      
      # Start of left intersection
      if dictItem["slp"] == 1:
        if leftFlag: # We seeing a new start without closing old end
          processValueErrorHelper("slp",sensorPose,gv.errorList,"Open slp record")
        else:
          leftFlag     = True
          leftDictItem = sensorPose.copy()

      # End of left intersection  NOTE the ending position should probably be adjusted by SENSORDISTANCEPASTTAPE
      # since it's past the tape when sensor goes off... do the same for ser
      if dictItem["sel"] == 1:
        if leftFlag == False:  # didn't see start of intersection
          processValueErrorHelper("sel",sensorPose,gv.errorList,"No preceeding slp record")
        else:
          leftFlag                = False
          tempDict                = sensorPose.copy()
          widthBetweenStartAndEnd = getDistanceBetweenNodes(leftDictItem,tempDict) 
          if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
            # Invalid tape width
            if widthBetweenStartAndEnd < gv.TAPEIGNOREWIDTH:
              print("** Saw tape noise, width {0}, was ignored".format(widthBetweenStartAndEnd))
            else:
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
          processValueErrorHelper("srp",sensorPose,gv.errorList,"Open srp record")
        else:
          rightFlag     = True
          rightDictItem = sensorPose.copy()

      # End of left intersection
      if dictItem["ser"] == 1:
        if rightFlag == False:  # didn't see start of intersection
          processValueErrorHelper("ser",sensorPose,gv.errorList,"No preceeding srp record")
        else:
          rightFlag               = False
          tempDict                = sensorPose.copy()
          widthBetweenStartAndEnd = getDistanceBetweenNodes(rightDictItem,tempDict) 
          if widthBetweenStartAndEnd < gv.MINTAPEWIDTH or widthBetweenStartAndEnd > gv.MAXTAPEWIDTH:
            # Invalid tape width
            if widthBetweenStartAndEnd < gv.TAPEIGNOREWIDTH:
              print("** Saw tape noise, width {0}, was ignored".format(widthBetweenStartAndEnd))
            else:
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

    # When down here the pose's have been adjusted

    # If error's we'll put the path we were just on back onto the list of paths to visit
    if len(gv.errorList) > 0:   
      # changed to use gv.pathBeingVisited
      path2Travel = gv.pathBeingVisited.copy()
      path2Travel["TYPE"] = "CORRECTION"
      path2Travel["INFO"] = "Had errors on path"
      #angle2Travel = utilities.getAngleAfterAdjustment(firstRobotPose["<"],180)    
      #path2Travel  = { "x" : firstRobotPose["x"], "y" : firstRobotPose["y"], "<" : angle2Travel, 
      #                  "TYPE" : "CORRECTION", "INFO" : "Had errors on path" }

      # $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ this needs to be adjusted it has sensorPose
      gv.paths2Visit.append(path2Travel)
    else:
      # No errors process the data in tempList

      # Use the robotPoseFromInstruction for lastRobotDictItem, this is needed because we need another node as the base to calculate
      # the distance being processed... mainly comes into play when calling exploration for the second time :)
      #lastRobotDictItem = gv.robotPoseFromInstruction.copy()  
      #tempNodeId        = getNodeAtPosition(lastRobotDictItem["x"],lastRobotDictItem["y"],False)
      #if tempNodeId != -1:
      #  gv.lastRobotDictItem["NODEID"] = tempNodeId

      writeHelper("--------------- P R O C E S S   V A L U E    L I S T (tempList below) ---------------\n")
      writeHelper("gv.lastRobotDictItem from last processValueList: {0} \n".format(str(gv.lastRobotDictItem)))
      writeHelper("Below is tempList (process value list items to process)")
      for robotDictItem in tempList:
        writeHelper("  robotDictItem: {0}".format(str(robotDictItem)))

      for robotDictItem in tempList:
        writeHelper("proc tempList, gv.lastRobotDictItem {0}".format(str(gv.lastRobotDictItem)))
        gv.worldXMin = min(gv.worldXMin, robotDictItem["x"])
        gv.worldXMax = max(gv.worldXMax, robotDictItem["x"])
        gv.worldYMin = min(gv.worldYMin, robotDictItem["y"])
        gv.worldYMax = max(gv.worldYMax, robotDictItem["y"])

        currNodeId = getNodeAtPosition(robotDictItem["x"],robotDictItem["y"])
        # Add the node id to the dict item, better than repeating everywhere below, note tempList will reflect
        # this as robotDictItem is a reference variable
        robotDictItem["NODEID"] = currNodeId
        gv.pathsVisited.append(robotDictItem.copy())

        if robotDictItem["TYPE"] == "sas":  # At start of maze
          if len(gv.startOfMaze) == 0:
            gv.startOfMaze = robotDictItem.copy()
            writeHelper("Found SAS: {0}".format(gv.startOfMaze))

        # At exit, add this to the list of potential goals
        if robotDictItem["TYPE"] == "sae":      
          gv.potentialGoalNodes.append(currNodeId)
          gv.potentialGoalDicts.append(robotDictItem.copy())
          nodeConnectionHelper(gv.lastRobotDictItem,robotDictItem)  # Add this node and the last one to the nodeConnectionList
          writeHelper("Found SAE: {0}".format(robotDictItem))

        # If it is a goal then set the goal node identifier
        if robotDictItem["TYPE"] == "sgl":        
          gv.goalDict = robotDictItem.copy()
          writeHelper("Found SGL: {0}".format(robotDictItem))

        if robotDictItem["TYPE"] == "sli" or robotDictItem["TYPE"] == "sri":   
          if (gv.DEBUGGING):
            writeHelper("calling nodeConnectionHelper: ")
            writeHelper(str(gv.lastRobotDictItem))
            writeHelper(str(robotDictItem))     
          nodeConnectionHelper(gv.lastRobotDictItem,robotDictItem)  
          intersectDict = robotDictItem.copy()
          # Want to put the destination angle into the < element, we'll
          # save orig< just in case :)
          intersectDict["orig<"] = intersectDict["<"]
          intersectDict["<"]     = intersectDict["dest<"]
          writeHelper("Found {0}: {1}".format(robotDictItem["TYPE"],intersectDict))

          # Check that it hasn't already been visited, if not add it to the list
          if hasPathBeenVisited(intersectDict) == False:
            writeSpeech("Added new path to visit")
            gv.paths2Visit.append(intersectDict.copy()) 
     
        # Save this item as the last one looked at if not sgl... sgl has sae also and we'll log that one
        if robotDictItem["TYPE"] != "sgl":
          gv.lastRobotDictItem = robotDictItem.copy()
  except:
    print("Exception raised - processValueList")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
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
        writeSpeech("State change")
        if gv.DEBUGGING: writeHelper("in setPathValueListFromString")
        stateObj = getState(arrayOfValues)
        gv.pathValueList.append(stateObj)
      
      if keyWord == "PO":  # This is still here but I moved the pose info into the IR record so 
        if gv.DEBUGGING:        #   it's probably not needed.... left in case :)
          writeHelper("In POSE processing")
        logPose = getPose(arrayOfValues)
        gv.pathValueList.append(logPose)
        if gv.DEBUGGING:
          writeHelper(logPose)
      if gv.DEBUGGING: writeHelper("Leaving setPathValueListFromString, len of array {0}".format(len(gv.pathValueList)))
  except:
    print("Exception raised - setPathValueListFromString")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# The sparki can't handle a lot of instructions (i.e. if string > 128 bytes) so this little
# helper will only send up to (3-parm) instructions at a time... the remaining instructions
# are put into 'gv.pendingInstructions' and the next time the sparki asks for instructions
# it'll send those
# You can call this with a set of instructions, or "" if you know there are pending ones and
# want to send them.
def tellSparkiHelper(currentInstructions, maxInstructions=3):
  if len(gv.pendingInstructions) == 0:
    gv.pendingInstructions = currentInstructions
  elif len(currentInstructions) > 0:
    gv.pendingInstructions += "," + currentInstructions
  # We will at most send maxInstructions at once to the sparki
  tempInstructions = gv.pendingInstructions.split(',')
  numInstructions = int(len(tempInstructions)/7)
  startIdx = 0
  endIdx = (min(numInstructions,maxInstructions) * 7) # this is one past the end of element wanted
  instruction2Return = ""
  while (startIdx < endIdx):
    instruction2Return += "," + tempInstructions[startIdx]
    startIdx += 1
  if len(instruction2Return) > 0:
    instruction2Return = instruction2Return[1:] # strip off first ,
  # Now reset the gv.pendingInstructions to be the remaining ones
  newPending = ""
  endIdx = numInstructions * 7
  while (startIdx < endIdx):  # startIdx is at the start of next block of instructions already
    newPending += "," + tempInstructions[startIdx]
    startIdx += 1
  if len(newPending) > 0:
    newPending = newPending[1:]
  gv.pendingInstructions = newPending
  return instruction2Return

# --------------------------------------------------------------------------------------------------
# Sparki wants instructions; if there's paths to visit then give him the best one
# if there aren't any paths then start checking goals
def tellSparkiWhatToDo():
  try:
    writeHelper("In tellSparkiWhatToDo (TS)")
    if len(gv.pendingInstructions) > 0:  # If there are pending instructions send them
      print("Sending pending instructions")
      return tellSparkiHelper("")

    movementType = gv.C_EXPLORE
    nodes2Target, gv.pathBeingVisited = getNextPath2Visit()
    if len(gv.pathBeingVisited) == 0:  # No paths left to visit see if there's a goal
      # See if we should save the map (we started in explore mode and haven't saved before)
      if gv.originalMode == gv.C_EXPLORE and gv.savedPickleMap == False:
        zSaveMapElementsToPickle()
        gv.savedPickleMap = True
        writeSpeech("Done exploring map")

      print("TSW2D: pathBeingVisited is empty, check goal")
      if gv.goalFound == ' ':
        movementType = gv.C_GOAL
        nodes2Target, gv.pathBeingVisited = getNextGoal2Visit()
      else:
        print("gv.goalFound is: {0}".format(gv.goalFound))

    if len(gv.pathBeingVisited) == 0: # No nodes or goals to vist tell em we're done
      writeSpeech("All done")
      return gv.C_DONE + ",x,-1.0,y,-1.0,<,-1"  # Need to pass all the elements that sparki expects, it'll ignore them"
    else:
      if movementType == gv.C_EXPLORE:
        writeSpeech("Exploration will start at position {0} and {1}".format(gv.pathBeingVisited["x"], \
                                                                                   gv.pathBeingVisited["y"]))
      else:       
        writeSpeech("Searching for goal at node: {0}".format(gv.pathBeingVisited["NODEID"]))
      return tellSparkiHelper(getDirectionsForNodes(nodes2Target, gv.pathBeingVisited, movementType))
  except:
    print("Exception raised - tellSparkiWhatToDo")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Routine to write out a string to the file that will be spoken (have speakTextFileContents.py running
# in another terminal session)
def writeSpeech(stringToSpeak):  
  try:
    if gv.writeSpeechFile > 0:
      if gv.writeSpeechFile == 1:  # First time call... create output file with header
        if utilities.fileExists(gv.speechFile):
          utilities.fileArchive(gv.speechFile)
    
        gv.speechFileHandle = open(gv.speechFile,"w")
        gv.writeSpeechFile  = 2
    gv.speechFileHandle.write(stringToSpeak + "\n")
    gv.speechFileHandle.flush()
  except:
    print("Exception raised - writeSpeech")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()

# --------------------------------------------------------------------------------------------------
# Write all the variables of interest out to the console
def writeHelper(theString):
  print(theString)
  gv.logFile.write(theString+"\n")
# --------------------------------------------------------------------------------------------------
# Write all the variables of interest out to the console
def writeVariables():
  try:
    writeHelper("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -")
    writeHelper("gv.startOfMaze (start position): {0}".format(str(gv.startOfMaze)))
    writeHelper("World xMin: {0} yMin: {1} xMax: {2} yMax: {3}".format(gv.worldXMin,gv.worldYMin,gv.worldXMax,gv.worldYMax))
    
    writeHelper("gv.currentMode: {0}".format(gv.currentMode))
    writeHelper("gv.lastRobotDictItem: {0}".format(gv.lastRobotDictItem))
    writeHelper("gv.robotPoseFromInstruction: {0}".format(str(gv.robotPoseFromInstruction)))
  
    writeHelper("gv.pathsVisited:")
    for aPathVisited in gv.pathsVisited:
      writeHelper("  {0}".format(str(aPathVisited)))
    
    writeHelper("gv.paths2Visit:")
    for aPath2Visit in gv.paths2Visit:
      writeHelper("  {0}".format(str(aPath2Visit)))
    
    writeHelper("pathBeingVisited")
    writeHelper("  {0}".format(str(gv.pathBeingVisited)))

    if len(gv.goalDict) > 0:
      writeHelper("gv.goalDict {0}".format(str(gv.goalDict)))
    else:
      writeHelper("gv.goalDict is empty")

    if gv.goalBeingChecked != ' ':
      writeHelper("gv.goalBeingChecked: {0}".format(gv.goalBeingChecked))
    if gv.goalFound != ' ':
      writeHelper("*** gv.goalFound: {0}".format(gv.goalFound))

    if len(gv.potentialGoalDicts) > 0:
      writeHelper("gv.potentialGoalDicts")
      for aPotentialGoal in gv.potentialGoalDicts:
        writeHelper("  {0}".format(str(aPotentialGoal)))        
    else:
      writeHelper("gv.potentialGoalDicts is empty")

    if len(gv.nodeList) > 0:
      writeHelper("gv.nodeList")
      for nodeId in range(len(gv.nodeList)):
        writeHelper("  NodeId: {0} value: {1}".format(nodeId,str(gv.nodeList[nodeId])))
    
    if len(gv.nodeConnectionList) > 0:
      writeHelper("gv.nodeConnectionList")
      for aNodeConnection in gv.nodeConnectionList:
        writeHelper("  {0}".format(str(aNodeConnection)))
      writeHelper("  As a graph: {0}".format(str(getGraphOfNodeConnectionList())))
    
    writeHelper("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -")

  except:
    print("Exception raised - writeVariables")
    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print(exc_type, fname, exc_tb.tb_lineno)
    sys.exit()
  
# --------------------------------------------------------------------------------------------------
# Load the map elements from the pickle file
def zLoadMapElementsFromPickle(pickleFile=gv.pickleWithMap):
  if utilities.fileExists(pickleFile):
    pickle_out = open(pickleFile,"rb") 
    gv.worldXMin, gv.worldXMax, gv.worldYMin, gv.worldYMax, gv.startOfMaze, \
      gv.nodeList, gv.nodeConnectionList, gv.potentialGoalNodes, \
      gv.potentialGoalDicts, gv.pathsVisited = pickle.load(pickle_out)
    pickle_out.close()

# --------------------------------------------------------------------------------------------------
# Load the goal dictionary element from the pickle file
def zLoadGoalElementFromPickle(pickleFile=gv.pickleWithGoal):
  if utilities.fileExists(pickleFile):
    pickle_out = open(pickleFile,"rb") 
    gv.goalDict = pickle.load(pickle_out)
    pickle_out.close()  

# --------------------------------------------------------------------------------------------------
# Save elements to the pick file
def zSaveMapElementsToPickle(pickleFile=gv.pickleWithMap):
  pickle_out = open(pickleFile,"wb") 
  pickle.dump([gv.worldXMin, gv.worldXMax, gv.worldYMin, gv.worldYMax, gv.startOfMaze, \
               gv.nodeList, gv.nodeConnectionList, gv.potentialGoalNodes, \
               gv.potentialGoalDicts, gv.pathsVisited], pickle_out)
  pickle_out.close()

# --------------------------------------------------------------------------------------------------
# Save elements to the pick file
def zSaveGoalElementToPickle(pickleFile=gv.pickleWithGoal):
  pickle_out = open(pickleFile,"wb") 
  pickle.dump(gv.goalDict, pickle_out)
  pickle_out.close()  
