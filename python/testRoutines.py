import os
import sys
import serial
import time
from datetime import datetime
import pickle

import sharedVars as gv
import sparkiStats

def testGetPose():
  stringFromSparki = "PO,x,13.71,y,-0.96,<,177"
  arrayOfValues    = stringFromSparki.split(",")
  logPose = sparkiStats.getPose(arrayOfValues)
  print(logPose)


testGetPose()