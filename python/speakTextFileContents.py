import sys
import pyttsx3
import time
import sharedVars as gv
 
"""
I couldn't get pyttsx3 to work in it's own thread... I created this program
as an altenative... it continually will read a file and say the contents of it (it keeps
track of last record said), also it will stop after (seconds2Wait) of inactivity
Thought is have a program create a file and continually write to it, launch this
program in another session and pass in the name of the file to speak :)
Note: If you don't pass in the name of the file then it'll use gv.
"""
if len(sys.argv) >= 2:
  fileToSpeak = sys.argv[1]
else: 
  fileToSpeak = gv.speechFile
 
seconds2Wait = 120
lastRecord   = -1
lastTime     = time.time()
 
engine = pyttsx3.init()
voices = engine.getProperty('voices')
engine.setProperty('voice','com.apple.speech.synthesis.voice.samantha')
def speakIt(text2Speak):
 engine.say(text2Speak)
 engine.runAndWait()
 
speakIt("Processing {0}".format(fileToSpeak))
while (time.time() - lastTime) < seconds2Wait:
 try:
   with open(fileToSpeak) as f:
     for lineNo, lineOfText in enumerate(f):
       print("lineNo: {0} lineOfText: {1}".format(lineNo,lineOfText))
       if lineNo > lastRecord:
         speakIt(lineOfText)
         lastRecord = lineNo
         lastTime   = time.time() # Reset the counter
         #else:
         #  time.sleep(0.5)
 except:
   time.sleep(0.5)

speakIt("Been inactive for {0} seconds, shutting down".format(seconds2Wait))