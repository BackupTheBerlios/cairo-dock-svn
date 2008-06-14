#!/bin/bash
# Audacious Emulated pipe
# Pipe created by ChAnGFu

if [ ! -w $1 ] ; then exit 1 ; fi
FILE=$1

STATUS=$(audtool playback-status)
TITLE=$(audtool current-song)

if [  "$STATUS" = "audtool: audacious server is not running!" ]; then
  exit
fi
if [  "$TITLE" = "No song playing." ]; then
  exit
fi

#Status du player
echo "status: $STATUS" > $FILE
#Position du morceaux
echo "trackInPlaylist: $(audtool playlist-position)" >> $FILE
#Position actuelle en secondes
echo "uSecPosition: $(audtool current-song-output-length-frames)" >> $FILE
#Temps écoulé
#echo "timeElapsed $(audtool current-song-output-length)" >> $FILE
echo "" >> $FILE
#Temps total en secondes
echo "totalTimeInSec $(audtool current-song-length-frames)" >> $FILE
#Temps total du son
#echo "totalTime: $(audtool current-song-length)" >> $FILE
echo "" >> $FILE
#Titre du son
echo "nowTitle: $TITLE" >> $FILE
