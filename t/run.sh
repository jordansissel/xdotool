#!/bin/sh

if [ ! -z "$XSERVER" ] ; then
  ( 
    if [ ! -z "QUIET" ] ; then
      exec > /dev/null
      exec 2> /dev/null
    fi
    exec $XSERVER $TEST_DISPLAY $EXTRA 
  ) &
  serverpid=$!
  sleep 1

  if [ ! -z "$WM" ] ; then
    ( 
      if [ ! -z "QUIET" ] ; then
        exec > /dev/null
        exec 2> /dev/null
      fi
      export DISPLAY="$TEST_DISPLAY"
      exec $WM
    ) &
    wmpid=$!

    sleeptime=5
    # gnome-session takes a long time to start.
    [ "$WM" = "gnome-session" ] && sleeptime=15
    echo "Sleeping for a $sleeptime seconds to let the wm startup ($WM) [$wmpid]"
    sleep $sleeptime
  fi # if ! -z $WM

  [ -z "$KEYMAP" ] && KEYMAP=us
  echo "Setting up keymap on new server as $KEYMAP"
  DISPLAY="$TEST_DISPLAY" setxkbmap $KEYMAP
fi # if ! -z $XSERVER

if [ ! -z "$TEST_DISPLAY" ] ; then
  DISPLAY="$TEST_DISPLAY"
fi

export DISPLAY
ruby alltests.rb

exitstatus=$?

if [ ! -z "$wmpid" ] ; then
  kill -9 "$wmpid"
fi

if [ ! -z "$serverpid" ] ; then
  kill -9 "$serverpid"
fi

exit $exitstatus
