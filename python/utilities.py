import math

# ----------------------------------------------------------------------
# Calculate difference between two points 
def calculateDistanceBetweenPoints(x1,y1,x2,y2):
  deltaX = x2 - x1
  deltaY = y2 - y1
  return round(math.sqrt((deltaX*deltaX)+(deltaY*deltaY)),2)

# ----------------------------------------------------------------------
# Return the midpoint between two points (returns a tuple)
def getMidpointBetweenPoints(x1,y1,x2,y2):
  xMid = round(((x1 + x2)/2.0),2)
  yMid = round(((y1 + y2)/2.0),2)
  return ( xMid, yMid )

# ----------------------------------------------------------------------
# Takes in an angle and an adjustment (+ or -) and returns the resulting
# angle within 0->359' range
def getAngleAfterAdjustment(baseAngle, adjustment):
  return (360 + (baseAngle + adjustment))%360  