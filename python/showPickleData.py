import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats
import utilities
from pandas import DataFrame

createMapData = True

gv.logFile = open("pickleLogRun.txt","at") # Append and text file
if utilities.fileExists(gv.pickleWithMap):
  print("pickle file exists")
  gv.originalMode = 'G'
  sparkiStats.zLoadMapElementsFromPickle()
  sparkiStats.zLoadGoalElementFromPickle()

sparkiStats.writeVariables()

if createMapData == True:
  data = {"x" : [], "y" : []}
  """
  for idx in range(len(gv.nodeList)):
    data["x"].append(gv.nodeList[idx]["x"])
    data["y"].append(gv.nodeList[idx]["y"])
  """  
  for aPath in gv.pathsVisited:
    data["x"].append(aPath["x"])
    data["y"].append(aPath["y"])
  
  df = DataFrame(data, columns= ['x', 'y'])
  export_csv = df.to_csv ('data.csv', index = None, header=["x_value","y_value"])