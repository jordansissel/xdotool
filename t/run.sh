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

# Give gnome a few seconds to get going...
sleep 5

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
kill $gnome_pid $xvfb_pid

exit $exitcode
