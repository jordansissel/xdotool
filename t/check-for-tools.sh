#!/bin/sh

XSERVERNAME=${XSERVER%% *}
WMNAME=${WM%% *}

if ! which $XSERVERNAME > /dev/null 2>&1 ; then
  echo "Can't find $XSERVERNAME, skipping..." >&2
  exit 1
fi

if [ ! -z "$WMNAME" -a "$WMNAME" != "none" ] \
   && ! which $WMNAME > /dev/null 2>&1 ; then
  echo "Can't find $WMNAME, skipping..." >&2
  exit 1
fi
