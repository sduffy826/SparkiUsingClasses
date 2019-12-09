import pyttsx3
engine = pyttsx3.init()
voices = engine.getProperty('voices')

def testIt(voiceId):
  engine.setProperty('voice',voiceId)
  engine.say('Node 0  Adjust Pose  Exploring 25.2 to 8.03')
  engine.runAndWait()
  engine.stop()
"""
for voice in voices:
    print("voice {0}".format(str(voice)))
    print("voice.id {0}".format(str(voice.id)))
    #print(voice, voice.id)
    engine.setProperty('voice', voice.id)
    engine.say("This is a test of some text")
    engine.runAndWait()
    engine.stop()
"""
testIt('com.apple.speech.synthesis.voice.karen')
testIt('com.apple.speech.synthesis.voice.Fred') 
testIt('com.apple.speech.synthesis.voice.samantha') 
testIt('com.apple.speech.synthesis.voice.Alex')    

