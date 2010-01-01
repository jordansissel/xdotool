#!/bin/sh

if ! which Xvfb > /dev/null 2>&1 ; then
  echo "Xvfb not found, but it is needed for the tests."
  exit 1
fi

export DISPLAY=:15
Xvfb $DISPLAY &
xvfb_pid="$!"

gnome-session > /dev/null 2>&1 &
gnome_pid="$!"
sleep 5

sh no_crashes_please.sh
sh test_search_maxdepth.sh
sh test_set_window.sh
kill $gnome_pid $xvfb_pid
