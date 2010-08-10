#!/bin/sh

if [ -z "$WINDOWID" ] ; then
  echo "No WINDOWID found in env? (xterm and gnome-terminal set this)"
  exit 1
fi


# Reparent our window to the nautilus desktop window
xdotool search --classname desktop_window \
    windowreparent $WINDOWID %1 \

# Change size
xdotool windowsize $WINDOWID 100% 10%

# Set behaviors
xdotool behave $WINDOWID mouse-enter windowfocus windowsize 100% 50% &
xdotool behave $WINDOWID mouse-leave windowsize 100% 10% &

