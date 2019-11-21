# This file has the global variables shared amonst modules

# Constants used to communicate with Sparki, these are from python to sparki
# Make sure the values below match what the sparki expects
C_EXPLORE = "X"
C_GOAL    = "G"
C_DONE    = "Q"  # quit
C_GOTO    = "M"  # move to

# These are shared variables across different sparki modules
DEBUGGING    = False
TAPEWIDTH    = 4.9 # 2.5 # 4.7
MINTAPEWIDTH = TAPEWIDTH - 0.5
MAXTAPEWIDTH = TAPEWIDTH + 0.5

SERIALTERMINATOR = "|"

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

startDict          = {} # The starting position, also has NODEID which is the node id
goalDict           = {} # The current goal, it has NODEID too
closestNodePose    = {} # The closestNodePose to the current position (finalPose)
finalPose          = {} # This is the last pose the robot reading took, we'll want to move from
                        #   this position to closestNodePose to get it back to a node position
potentialGoalNodes = []
potentialGoalDicts = []
goalsVisited       = [] # Goal nodes that have been visited
paths2Visit        = [] # Paths that need to be visited
pathsVisited       = [] # The paths that have been visited
pathBeingVisited   = {} # Path being visited