#!/bin/sh
# On my workstation, this switches to desktop 2, clears the search bar, pastes
# into the search bar, and presses enter. Hence searching for whatever lies in
# my X clipboard.

xdotool key "alt+2"
xdotool key "ctrl+j"
xdotool key "BackSpace"
xdotool mousemove 600 35
xdotool click 2
xdotool key "Return"
