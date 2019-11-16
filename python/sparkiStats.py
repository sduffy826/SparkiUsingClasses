
from statistics import mean, median, mode, stdev

"""
IA,FollowTape
IR,Base,el,974,ll,976,lc,978,lr,974,er,968,ell,0,lll,0,lcl,0,lrl,0,erl,0,sdl,0,sdr,0,sol,0,sae,0,slp,0,srp,0,sel,0,ser,0,sas,0
IR,last,el,974,ll,976,lc,978,lr,974,er,968,ell,0,lll,0,lcl,0,lrl,0,erl,0,sdl,0,sdr,0,sol,0,sae,0,slp,0,srp,0,sel,0,ser,0,sas,0
IR,StateChg,el,272,ll,581,lc,659,lr,693,er,272,ell,1,lll,1,lcl,1,lrl,1,erl,1,sdl,0,sdr,0,sol,1,sae,0,slp,1,srp,1,sel,0,ser,0,sas,0
PO,x,5.34,y,0.00,<,0
IR,StateChg,el,893,ll,151,lc,174,lr,115,er,884,ell,0,lll,1,lcl,1,lrl,1,erl,0,sdl,0,sdr,0,sol,1,sae,0,slp,0,srp,0,sel,1,ser,1,sas,0
PO,x,10.07,y,0.00,<,0
IR,StateChg,el,919,ll,156,lc,178,lr,135,er,573,ell,0,lll,1,lcl,1,lrl,1,erl,1,sdl,0,sdr,0,sol,1,sae,0,slp,0,srp,1,sel,0,ser,0,sas,0
PO,x,24.83,y,0.00,<,0
IR,StateChg,el,159,ll,267,lc,286,lr,262,er,114,ell,1,lll,1,lcl,1,lrl,1,erl,1,sdl,0,sdr,0,sol,1,sae,0,slp,1,srp,0,sel,0,ser,0,sas,0
PO,x,25.55,y,0.00,<,0
IR,StateChg,el,970,ll,114,lc,115,lr,89,er,970,ell,0,lll,1,lcl,1,lrl,1,erl,0,sdl,0,sdr,0,sol,1,sae,0,slp,0,srp,0,sel,1,ser,1,sas,0
PO,x,30.29,y,0.00,<,0
IR,StateChg,el,969,ll,200,lc,213,lr,172,er,341,ell,0,lll,1,lcl,1,lrl,1,erl,1,sdl,0,sdr,0,sol,1,sae,0,slp,0,srp,1,sel,0,ser,0,sas,0
PO,x,45.05,y,0.00,<,0
Done
"""

def setVars(sensorList, stringFromSparki):
  # Store values... later we'll do analysis on them
  arrayOfValues = stringFromSparki.split(",")
  if len(arrayOfValues) > 2:  # Has to has more than two values
    keyWord = arrayOfValues[0]
    if keyWord == "IR" and arrayOfValues[1] == "StateChg":
      print("in IR processing")
      stateObj = getState(arrayOfValues)
      sensorList.append(stateObj)
    
    if keyWord == "PO":
      print("In POSE processing")
      logPose = getPose(arrayOfValues)
      print(dict(logPose))
      sensorList.append(logPose)
      print("Len of array {0}".format(len(sensorList)))

# Return dictionary for the state
def getState(arrayValues):
  idx = 3 # Have it represent where the value is not the label for it
  stateToReturn = { "RECTYPE" : "STATE" }
  while idx <= len(arrayValues):
    sensorLabel = arrayValues[idx]
    sensorValue = arrayValues[idx+1]
    idx += 2
    stateToReturn[sensorLabel] = sensorValue
  return stateToReturn

# Return a dictionary for the pose
def getPose(arrayValues):
  print("in getPose, len of args: {0}".format(len(arrayValues)))
  poseToReturn = { "RECTYPE" : "POSE" }
  idx = 1
  while (idx <= len(arrayValues)):
    print("getPose key: {0} value: {1}".format(arrayValues[idx],arrayValues[idx+1]))
    poseToReturn[arrayValues[idx]] = arrayValues[idx+1]
    print(poseToReturn)
    idx += 2
  
  print("Down here")
  print("poseToReturn ")
  print(poseToReturn)
  return poseToReturn
