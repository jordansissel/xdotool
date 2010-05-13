#!/bin/sh

[ -z "$KEYMAP" ] && KEYMAP=us
echo "Setting up keymap on new server as $KEYMAP"
setxkbmap $KEYMAP

# Add local built libxdo.so
export LD_LIBRARY_PATH="${PWD}/.."

"$@"
exitstatus=$?

exit $exitstatus
