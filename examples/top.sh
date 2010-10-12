#!/bin/sh
# Similar to leftconsole, but activates a window when you hit the top
# of the screen

position=top
classname="xdotool_slidein_$position"

# restore the previous window if there was one.
xdotool search --classname "$classname" \
  set_window --classname "nothing" \
  set_window --overrideredirect 0 windowunmap windowmap

# Select a window and tag it with a special classname
echo "Click on any window to have it slide in when you move the mouse to the $positoin"
xdotool selectwindow set_window --classname "$classname"
echo "Window selected. Now move the mouse to the $position part of the screen to activate it."

# We have to make the windowmanager ignore this window so it doesn't get
# handled like a normal client would (no window borders/decorations, etc)
# To make this change, we have to unmap and remap the window.
xdotool search --classname "$classname" \
  set_window --overrideredirect 1 \
  windowunmap windowmap

# Set up a behavior for the left edge. Maximize the window vertically
# And since I generally use an xterm, make the width 80 characters wide.
xdotool behave_screen_edge --quiesce 0  top \
  search --classname "$classname" \
  windowmap \
  windowfocus \
  windowraise \
| xdotool search --classname "$classname" behave %@ mouse-leave windowunmap \

