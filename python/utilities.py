import math

# Constant for the maximum delta between two angles to still consider
# them to be in the same direction
MAXANGLEDELTASTOBECONSIDEREDSAME = 14

# ----------------------------------------------------------------------
# Helper to say that two angles are close enough (i.e. 88 and 91 are 
# pointing down the same path)
def areAnglesCloseEnough(angle1, angle2):
  print("In are<Close {0} and {1}".format(angle1,angle2))
  return (abs(angle1 - angle2) <= MAXANGLEDELTASTOBECONSIDEREDSAME)  


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
# Return the midpoint between two points (returns a tuple)
def getMidpointBetweenPoints(x1,y1,x2,y2):
  xMid = round(((x1 + x2)/2.0),2)
  yMid = round(((y1 + y2)/2.0),2)
  return ( xMid, yMid )
