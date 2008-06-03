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

sleep 1

try $xdotool search xdotool_test_window
try $xdotool getwindowfocus

wid=`$xdotool search xdotool_test_window`
try $xdotool windowraise $wid
try $xdotool windowsize $wid 500 500
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

try $xdotool windowactivate $wid
sleep 0.2

try $xdotool get_num_desktops
desktops=`$xdotool get_num_desktops`
try $xdotool set_num_desktops $desktops

cur_desktop=`$xdotool get_desktop`
try $xdotool set_desktop $cur_desktop

desktop=`$xdotool get_desktop_for_window $wid`
try $xdotool set_desktop_for_window $wid $desktop

#pkill -f xdotool_test_window
kill $xterm_pid
