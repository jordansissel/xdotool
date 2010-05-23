#!/bin/sh
# Start an ephemeral X server.
#
# This is useful for when you want to lauch an X server for a specific
# process. When that process exits, the X server will be killed.
# 

XSERVER=Xvfb
WINMGR=

prog=$0
usage() {
  echo "Usage: $prog [-x XSERVER] [-w WINDOWMANAGER] [-q] [-h] <command>"
  echo "-h   this help"
  echo "-q   quiet"
  echo "-w   window manager process to start once Xserver is up"
  echo "     (default: '$WINMGR')"
  echo "-x   Xserver (and args) to run"
  echo "     (default: '$XSERVER')"
  echo
  echo "This tool will pick an unused DISPLAY value (:0, :1, etc) and"
  echo "start an Xserver on that display, then run your command."
  echo
  echo "Examples:"
  echo "  $prog -x 'Xephyr -screen 1280x720' xterm"
  echo "  $prog -x 'Xvnc -httpd /usr/share/vnc/classes -geometry 1024x768 -depth 24' -w "gnome-session" firefox"
}

quiet() {
  [ "0$QUIET" -eq 1 ]
}

cleanup() {
  if [ ! -z "$winmgrpid" ] ; then
    kill -TERM "$winmgrpid" || true
  fi
  kill -TERM "$xpid" || true

  pkill -KILL -P $$ || true
}


eval "set -- $( (getopt -- x:w:qh "$@" || echo " "FAIL) | tr -d '\n')"

while [ "0$#" -gt 0 ] ; do
  case $1 in
    -x) XSERVER=$2; shift ;;
    -w) WINMGR=$2; shift ;;
    -q) QUIET=1 ;;
    -h) usage; exit ;;
    --) shift; break ;;
  esac
  shift
done

if [ "$1" = "FAIL" ] ; then
  usage
  exit 1
fi

num=-1
XSERVERNAME=${XSERVER%% *}
if ! which "$XSERVERNAME" > /dev/null 2>&1 ; then
  echo "Unable to find $XSERVERNAME. Aborting."
  exit 1
fi

if ! which lsof > /dev/null 2>&1 ; then
  echo "Unable to find lsof. This is a required tool."
  exit 1
fi

while true; do 
  num=$(expr $num + 1)
  xsocket=/tmp/.X11-unix/X$num
  quiet || echo "Trying :$num"
  lsof $xsocket > /dev/null 2>&1 && continue
  (
    if quiet ; then
      exec > /dev/null
      exec 2> /dev/null
    fi
    set -- $XSERVER
    cmd=$1
    shift
    exec $cmd :$num "$@"
  ) &
  xpid=$!

  healthy=0
  for i in 1 2 3 4 5 6 7 8 9 ; do
    # Break early if the xserver died
    #ps -p $xpid > /dev/null 2>&1 || break
    kill -0 $xpid > /dev/null 2>&1 || break

    # See if the xserver got a hold of the display socket.
    # If so, the server is up and healthy.
    if lsof -p $xpid | grep -qF $xsocket ; then
      quiet || echo "$XSERVERNAME looks healthy. Moving on."
      healthy=1
      break
    fi
    sleep 0.2 || sleep 1 # In case your sleep doesn't take subsecond values
  done

  if [ "0$healthy" -eq 1 ] ; then
    break
  fi
done

export DISPLAY=:$num
quiet || echo "Using display: $DISPLAY"

if [ ! -z "$WINMGR" ] ; then
  if ! which $WINMGR > /dev/null 2>&1 ; then
    echo "Cannot find $WINMGR. Aborting."
    exit 1
  fi
  WINMGRNAME=${WINMGR%% *}
  quiet || echo "Starting window manager: $WINMGRNAME"
  (
    if quiet ; then
      exec > /dev/null
      exec 2> /dev/null
    fi
    $WINMGR
  ) &
  winmgrpid=$!

  # Wait for the window manager to startup
  quiet || echo "Waiting for window manager '$WINMGRNAME' to be healthy."
  # Wait for the window manager to start.
  for i in 1 2 3 4 5 6 7 8 9 10 ABORT ; do 
    # A good signal that the WM has started is that the WM_STATE property is
    # set or that any NETWM/ICCCM property is set.
    if xprop -root | egrep -q 'WM_STATE|^_NET' ; then
      quiet || echo "$WINMGRNAME looks healthy. Moving on."
      break;
    fi
    sleep .5

    if [ "$i" = "ABORT" ] ; then
      quiet || echo "Window manager ($WINMGRNAME) seems to have failed starting up."
      cleanup
      exit 1
    fi
  done
fi

quiet || echo "Running: $@"
(
  "$@"
)
exitcode=$?
cleanup
exit $exitcode
