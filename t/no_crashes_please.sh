#!/bin/sh

try () {
  "$@" > /dev/null
  
  if [ $? -ne 0 ]; then
    echo "FAILURE: $@"
  else
    echo "SUCCESS: $@"
  fi
}

make -C ../ clean xdotool
if [ $?  -ne 0 ] ; then
  echo "Failure building xdotool."
  exit 1
fi

xdotool="../xdotool"

xterm -T xdotool_test_window -e 'sleep 300' &
xterm_pid="$!"

try $xdotool search xdotool_test_window
try $xdotool getwindowfocus

wid=`$xdotool search xdotool_test_window`
try $xdotool windowsize $wid 50 50
try $xdotool windowfocus $wid
try $xdotool windowmove $wid 300 300
try $xdotool windowunmap $wid
try $xdotool windowmap $wid

try $xdotool mousedown 1
try $xdotool mouseup 1

try $xdotool mousemove 0 0
try $xdotool mousemove 50 50

try $xdotool click 1

try $xdotool type "hello"
try $xdotool key "ctrl+w"

pkill -f xdotool_test_window
