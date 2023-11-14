# Podcast-Player-With-Auto-Timer
 Night Time Podcast Player By John Melki-Wegner
   Has an issue with using
   DFPlayer.fastForward() and
   DFPlayer.setPlayTime()
   Both are unreliable when values of seconds are bigger than one byte!!!
   Supposed to be okay for unsigned int.
   Compromise:
   DFPlayer.fastForward() through file in increments of 200 to place the playhead where I want it
