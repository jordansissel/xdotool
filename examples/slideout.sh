#!/bin/sh

wid=$(xdotool search --classname "$1")
export wid
seq 0 20 | xargs -n1 sh -c "xdotool windowmove $wid $(($1 * -30)) 0" -
xdotool windowunmap "$wid"