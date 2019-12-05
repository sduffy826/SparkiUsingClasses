
<h2>Logic for mapping/searching maze</h2>
<p>Due to memory limitations on the sparki the mapping/goal searching is composed of two
   parts.  The sparki (part1) communicates over the serial interface to a python program (part2);
   the python program collects sparki readings and builds a map of the workspace.  When the sparki needs to
   determine what action to take it asks the python program for instructions.   The python
   routines determine what the next action should be based on the mode the sparki is in, and
   where the closest next node is.  There will be examples of how this works down below.</p>
<p>The robot has two mode's 'eXplore' and 'Goal'; explore mode is for mapping the maze.  
   During this phase it will record where the goal is, if one is found.  Once it's mapped the 
   entire workspace the python code will serialize the map so that future runs can be taken
   without having to go thru the explore mode.  The mode will then switch to 'Goal' mode;
   during this phase it will go to the last goal seen; if the goal is found the program
   will record the goal position and stop; if a goal is not located there it will search
   the next closest endpoint to see if the goal is there... it will repeat this process till it
   finds the goal, or all 'end point' nodes have been visited.</p>
<p>The program <strong>infraredClassTest.ino</strong> has the sparki code to execute... there 
   are a bunch of different routines within this ino file.  You control which ones you want to execute
   based on #define's.<br />
   The main one you want is the #define TESTMAZE, a lot of the others
   are related to testing 'line centering' (you can play with those if you like).</p>
<p>The python program <strong>serialProcessor.py</strong> is the mainline python program associated
   with the sparki code.  This program has several other 'include' files that support it</p>
<p>Python program <strong>plotXYRealtimeAnimation.py</strong> is a python program which will map
   the sparki as it moves around it's workspace.  In order to use this you need to have the
   sparki send it's pose to the serialProcessor.py program.  In the sparki code (infraredClass.h)
   you need to make sure you set the following #define to true<pre>
     #define WRITEMAPDATA true</pre>
   In the file 'sharedVars.py' also make sure you have the following var is set to 1, 0 means
   don't write the data.  (fyi I use int instead of boolean because 1 means you want data but  
   the output file hasn't been created yet... the code sets it to 2 once the output file has 
   been created)<pre>
     writeCSVData = 1</pre>
   To clarify... turning on WRITEMAPDATA causes the sparki to send it's pose over the serial line
   to 'serialProcessor.py'.  If it's variable 'writeCSVData=1' then it will take those values, reformat
   them and write them to a csv file.  You can then have the plotXYRealtimeAnimation.py program running in
   another session... it reads the values from the csv file and will display a map of the sparki's movements.<br />
   Note: you can always run this program after the simulation but it's more fun to watch the map 
   changing in real time.</p>
   
<p>To run the simulation do the following
  <ul> 
    <li>Assuming you're using VS code... if not don't worry about clearing terminal</li>
    <li>From terminal window make sure you're in the python directory</li>
    <li>Hit COMMAND+k to clear terminal window</li>
    <li>type 'python3 serialProcessor.py' (but don't hit enter)</li>
    <li>If you want to display the map of a run then launch another terminal window and type 
        'python3 plotXYRealtimeAnimation.py' (again don't hit enter)</li>
    <li>Place sparki at start of maze</li>
    <li>From arduino ide, compile/load the infraredClassTest.ino file onto the sparki</li>
    <li>After the program is loaded the sparki will beep</li>
    <li>Go to VS Code terminal window and hit enter to run the 'serialProcessor.py' program, if 
        map data has been serialized before then it'll ask you which mode you want to run it, 
        press X for eXplore or G for goal.</li>
    <li>If you want the map displayed then you can launch the plotXYRealtimeAnimation.py program,
        but wait till after the sparki has started moving forward before you do (you don't have
        to attempt to read the csv file before it has been opened for writing by the
        serialProcessor code)</li>
  </ul>
</p>

<h2>High level flow</h2>
<p>Sparki makes a 'handshake' with the python program and asks it the mode it should run in.
   If the python has a serialized map it will ask you (the human) what mode to run in, otherwise
   if no map data exists it'll tell the sparki to run in eXplore mode.  NOTE, the handshake
   must occur within 10 seconds... if that's not enough time then change the programs :) (or
   be quicker).</p>
<p>Instead of trying to describe the logic I'll show it in psuedo code.  Some notes
   when reading.
   <ul>
     <li>The instruction python returns is in the following format:
       <table>
         <tr>
           <td>Instruction</td>
             <td>Character for instruction (this is defined in infraredClass.h and sharedVars.py), they are:
                'X'plore, 'M'ove, 'G'oal, 'P' - set pose, 'D'one </td>
         </tr>
         <tr><td>x</td><td>The x position (float value)</td></tr>
         <tr><td>y</td><td>The y position (float value)</td></tr>
         <tr><td>x</td><td>The angle (int value)</td></tr>
       </table>
     </li>
   </ul>
</p>
<p>Here's the <strong>psuedo</strong> for the mainline program.</p>
<pre>
distance2Travel = INFRARED_MAX_DISTANCE_TO_TRAVEL
instruction = mode from handshake
Move forward distance2Travel while not done
  if we're mapping data (WRITEMAPDATA) then send robot pose down serial line
  Take sensor readings and see if the 'state' has changed from prior reading
  if instruction is new
    send instruction 'start', sensor pose down serial port
  if we haven't adjusted our position on this line
    call routine (adjustToTape) to reposition us on the tape (it returns a bool whether we adjusted or not)
    loop back to the top
  If state changed then
    if at end of tape 
      wait4Instruction &lt;- true 
      check4Obstacle &lt;- true 
    else if just past an intersection (left or right) 
      if no line in front of intersection 
        wait4Instruction &lt;- true 
    else if driftedLeft or driftedRight (n/a for duct tape... too wide)
      call routine (adjustForDrifting) (will move angle back torward center of tape)    
    
    if check4Obstacle 
      stop moving 
      obstacle &lt;- (ultrasonicReading < MAX_GOAL_DISTANCE)
    if mode is eXplore mode
      send following info down serial port: 'StateChange', IR attributes, SensorPose, obstacle
    if notMoving 
      wait4Instructions &lt;- true 
    if wait4Instructions
      stop moving 
      send following down serial port 'IR,INSTOP', instruction, poseOfSensor, obstacle
      if queueOfInstructions is empty 
        wait for python to give instructions and put them in 'queueOfInstructions'
      if queueOfInstructions is empty (meaning didn't get any from above)
        tell python program we're DONE 
        done &lt;- true
      else
        currentInstruction = queueOfInstructions.pop()  // This has instruction (Move,Xplore...) and pose 
        switch (currentInstruction.instruction) 
          case EXPLORE_MODE
            turnTo currentInstruction.pose.angle
            distance2Travel = INFRARED_MAX_DISTANCE_TO_TRAVEL
          case GOAL_MODE:                  
            turnTo currentInstruction.pose.angle 
            distanceToTravel = 0.0  // will ensure we're not moving and will cause reading to be taken
          case DONE_MODE:
            distanceToTravel = 0.0;
            done = true;
          case GOTO_MODE:
            turn to angle currentInstruction.pose.angle
            distanceToTravel = distance between current pose and pose in 'currentInstruction'
          case SETPOSE_MODE:
            distanceToTravel = 0.0;  // don't want it to move
            localizationObj->setCurrentXPosition(currentInstruction.pose.xPos);
            localizationObj->setCurrentYPosition(currentInstruction.pose.yPos);
          
        // We changed position in switch statement, get the pose of the sensor
        poseOfSensor = infraredObj->getPoseOfCenterSensor();
    end of block for wait4Instructions
  end of block for state change
</pre>