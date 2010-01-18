#!/bin/sh

# so that xdotool uses the recently-build libxdo.so.0 during the
# tests:
LD_LIBRARY_PATH=$(pwd)/..
export LD_LIBRARY_PATH

# default to use Xephyr as xserver
: ${XSERVER=:Xephyr}

if ! which $XSERVER > /dev/null 2>&1 ; then
  echo "$XSERVER not found, but it is needed for the tests."
  exit 1
fi

_DISPLAY=:15

if which openbox-session > /dev/null 2>&1 ; then
    session="openbox-session"
else
    session="gnome-session"
fi

$XSERVER $_DISPLAY &
server_pid="$!"
export DISPLAY=$_DISPLAY
sleep 2

$session > /dev/null 2>&1 &
session_pid="$!"

# Give the session manager a few seconds to get going...
sleep 8

results=$(mktemp)
( sh no_crashes_please.sh
  sh test_search_maxdepth.sh
  sh test_set_window.sh
) | tee $results

if grep -q "^FAILURE:" $results ; then
  exitcode=1
else
  exitcode=0
fi

echo "$(grep -c "^FAILURE:" $results) tests failed"
echo "$(grep -c "^SUCCESS:" $results) tests passed"

rm $results
kill $server_pid $session_pid
wait

exit $exitcode
