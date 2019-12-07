# This file has the global variables shared amonst modules

# Constants used to communicate with Sparki, these are from python to sparki
# Make sure the values below match what the sparki expects
C_EXPLORE = "X"
C_GOAL    = "G"
C_DONE    = "Q"  # quit
C_GOTO    = "M"  # move to
C_SETPOSE = "P"  # set pose of robot

INFRARED_SENSOR_FORWARD_OF_CENTER = 4

# These are shared variables across different sparki modules
DEBUGGING    = False
TAPEWIDTH    = 4.7 # 2.5 # 4.7
MINTAPEWIDTH = TAPEWIDTH - 0.7
MAXTAPEWIDTH = TAPEWIDTH + 1.1

SERIALTERMINATOR = "|"

logFile = None

# Handle for writing csv file with poses...
csvFileName   = "data.csv"
csvFileHandle = None
csvFieldNames = ["x_value", "y_value"]
csvWriter     = None
writeCSVData  = 1  

# File for speaking
speechFile       = "serialProcessorSpeak.Me"
speechFileHandle = None
writeSpeechFile  = 1

# Pickle files to save variables.
pickleWithMap   = "pickleWithMap.bin"
savedPickleMap  = False
pickleWithGoal  = "pickleWithGoal.bin"
savedPickleGoal = False

# Map related elements, these should be saved as soon as you mapped the
# configuration space
# ------------------------------------------------------------------------------
# Init to values that you know are outside range of min/maxe's you'll see
worldXMin = 200.0
worldXMax = -200.0
worldYMin = 200.0
worldYMax = -200.0

startOfMaze        = {} # The starting position, also has NODEID which is the node id

nodeList           = [] # List of nodes
nodeConnectionList = [] # Nodes and node there connected, with angle and distance between them

potentialGoalNodes = [] # Potential goal nodeid's
potentialGoalDicts = [] # Potential goals with dict elements (x,y...)

pathsVisited       = [] # The paths that have been visited

# This should be saved as soon as you find the goal.
# ------------------------------------------------------------------------------
goalDict           = {} # The current goal, it has NODEID too


# Vars for the current run, these should stay 'initial'
# ------------------------------------------------------------------------------
originalMode     = ' ' # Original run mode (explore or goal), if explore then we'll
                       #   write out the map when we explored it
currentMode      = ' ' # Mode running (explore, goal, done, goto)
goalBeingChecked = ' ' # The goal node that's being checked
goalFound        = ' ' # When goal is found this has it's node id

goalsVisited     = [] # Goal nodes that have been visited
paths2Visit      = [] # Paths that need to be visited

lastRobotDictItem        = {}
robotPoseFromInstruction = {}

# Vars for exploration 
# ------------------------------------------------------------------------------
pathValueList = [] # The sparki gives us a bunch of values between start and end of a path, we
                   # store them here... when the path is at the end we process the values and
                   # update our attributes/environment
errorList     = [] # Work area to record errors that happend during a given path 
 
pathBeingVisited = {} # Path being visited

# Misc
pendingInstructions = '' # Can't send sparki too many instructions at once this is
                          # remaining instructions