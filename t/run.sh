#!/bin/sh

[ -z "$KEYMAP" ] && KEYMAP=us
echo "Setting up keymap on new server as $KEYMAP"
setxkbmap $KEYMAP

# Add local built libxdo.so
export LD_LIBRARY_PATH="${PWD}/.."

ruby alltests.rb 
exitstatus=$?

if [ ! -z "$wmpid" ] ; then
  kill -9 "$wmpid"
fi

if [ ! -z "$serverpid" ] ; then
  kill -9 "$serverpid"
fi

exit $exitstatus
