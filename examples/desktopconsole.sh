#!/bin/sh

if [ -z "$WINDOWID" ] ; then
  echo "No WINDOWID found in env? (xterm and gnome-terminal set this)"
  exit 1
fi


# Change size
xdotool windowsize $WINDOWID 100% 1

# Reparent our window to the nautilus desktop window
if xdotool search --classname desktop_window ; then
  xdotool search --classname desktop_window windowreparent $WINDOWID %1
else
  xdotool set_window --overrideredirect  1 $WINDOWID windowunmap $WINDOWID 
  xdotool windowmap $WINDOWID
fi

# Set behaviors
xdotool behave $WINDOWID mouse-enter windowfocus windowsize 100% 50% &
xdotool behave $WINDOWID mouse-leave windowsize 100% 1 &

