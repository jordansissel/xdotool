#!/bin/sh


WINDOWID=$(xdotool selectwindow)

xdotool set_window --overrideredirect 1 $WINDOWID windowunmap $WINDOWID windowmap $WINDOWID
xdotool windowsize $WINDOWID 10 100%

# Set behaviors
xdotool behave $WINDOWID mouse-enter windowfocus windowsize --usehints 80 100% &
xdotool behave $WINDOWID mouse-leave windowsize 4 100% &
