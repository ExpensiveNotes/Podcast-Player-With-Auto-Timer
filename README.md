# Podcast-Player-With-Auto-Timer
 Night Time Podcast Player By John Melki-Wegner

This program controls a DFRobot DFPlayer Pro via a Arduino Nano. It allows me to listen to a podcast at night with a pillow speaker by pressing one button to start the podcast at a set time and also set a 10 minute sleep timer automatically.
 
   Has an issue with using
   DFPlayer.fastForward() and
   DFPlayer.setPlayTime()
   Both are unreliable when values of seconds are bigger than one byte!!!
   Supposed to be okay for unsigned int.
   Compromise:
   DFPlayer.fastForward() through file in increments of 200 to place the playhead where I want it
