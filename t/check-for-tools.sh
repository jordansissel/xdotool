#!/bin/sh

XSERVERNAME=${XSERVER%% *}
WMNAME=${WM%% *}

need() {
  if ! which "$1" > /dev/null 2>&1 ; then
    echo "Can't find program '$1', skipping..." >&2
    exit 1
  fi
}

need "$XSERVERNAME"

if [ ! -z "$WMNAME" -a "$WMNAME" != "none" ]; then
  need "$WMNAME"
  exit 1
fi

need xwininfo
need xdpyinfo
need xprop
