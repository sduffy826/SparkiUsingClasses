# This file has the global variables shared amonst modules

# These are shared variables across different sparki modules
DEBUGGING    = False
TAPEWIDTH    = 4.7
MINTAPEWIDTH = TAPEWIDTH - 0.5
MAXTAPEWIDTH = TAPEWIDTH + 0.5

# Init to values that you know are outside range of min/maxe's you'll see
worldXMin = 200.0
worldXMax = -200.0
worldYMin = 200.0
worldYMax = -200.0



potentialGoals = []  # List of potential goals
startPosition  = {}  # Starting position 

pathValueList = [] # The sparki gives us a bunch of values between start and end of a path, we
                   # store them here... when the path is at the end we process the values and
                   # update our attributes/environment
errorList     = [] # Work area to record errors that happend during a given path 
 
nodeList           = [] # List of nodes
nodeConnectionList = [] # Nodes and node there connected, with angle and distance between them
nodesToVisit       = [] # Nodes to visit and the angle they should visit

startNode          = -1
startDict          = {}
goalNode           = -1
goalDict           = {}  # Pose of the goal
potentialGoalNodes = []
potentialGoalDicts = []
paths2Visit        = []  # Paths that need to be visited
pathsVisited   = []  # The paths that have been visited