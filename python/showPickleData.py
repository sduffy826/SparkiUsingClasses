import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats
import utilities

gv.logFile = open("pickleLogRun.txt","at") # Append and text file
if utilities.fileExists(gv.pickleWithMap):
  print("pickle file exists")
  gv.originalMode = 'G'
  sparkiStats.zLoadMapElementsFromPickle()
  sparkiStats.zLoadGoalElementFromPickle()

sparkiStats.writeVariables()
