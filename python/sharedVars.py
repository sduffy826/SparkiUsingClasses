# This file has the global variables shared amonst modules

# Constants used to communicate with Sparki, these are from python to sparki
# Make sure the values below match what the sparki expects
C_EXPLORE = "X"
C_GOAL    = "G"
C_DONE    = "Q"  # quit
C_GOTO    = "M"  # move to

INFRARED_SENSOR_FORWARD_OF_CENTER = 4

# These are shared variables across different sparki modules
DEBUGGING    = False
TAPEWIDTH    = 4.7 # 2.5 # 4.7
MINTAPEWIDTH = TAPEWIDTH - 0.7
MAXTAPEWIDTH = TAPEWIDTH + 1.1


SERIALTERMINATOR = "|"

logFile = None

# Init to values that you know are outside range of min/maxe's you'll see
worldXMin = 200.0
worldXMax = -200.0
worldYMin = 200.0
worldYMax = -200.0

currentMode      = ' ' # Mode running (explore, goal, done, goto)
goalBeingChecked = ' ' # The goal node that's being checked
goalFound        = ' ' # When goal is found this has it's node id

lastRobotDictItem = {}
# Start after rewor
robotPoseFromInstruction = {}
robotPoseAtStart = {}
robotPoseAtStop  = {}


sensorPoseAtStart = {}  # Pose at the start/end of an instruction sequence
sensorPoseAtStop  = {}

potentialGoals = []  # List of potential goals
startPosition  = {}  # Starting position 

pathValueList = [] # The sparki gives us a bunch of values between start and end of a path, we
                   # store them here... when the path is at the end we process the values and
                   # update our attributes/environment
errorList     = [] # Work area to record errors that happend during a given path 
 
nodeList           = [] # List of nodes
nodeConnectionList = [] # Nodes and node there connected, with angle and distance between them
nodesToVisit       = [] # Nodes to visit and the angle they should visit

startOfMaze        = {} # The starting position, also has NODEID which is the node id
goalDict           = {} # The current goal, it has NODEID too

potentialGoalNodes = []
potentialGoalDicts = []
goalsVisited       = [] # Goal nodes that have been visited
paths2Visit        = [] # Paths that need to be visited
pathsVisited       = [] # The paths that have been visited
pathBeingVisited   = {} # Path being visited