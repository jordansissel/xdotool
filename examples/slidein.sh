#!/bin/sh

export wid=$(xdotool search --classname "$1")
seq 20 -1 0 | xargs -n1 sh -c 'xdotool windowmove $wid $(($1 * -30)) 0' -

