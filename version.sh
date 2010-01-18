#!/bin/sh

  
MAJOR="0"
DATE="$(date +%Y%m%d)"
REVISION=$([ -d .svn ] && svn info . | awk '/Revision:/ {print $2}')
: ${REVISION=:0}

case $1 in
  --major) echo "$MAJOR" ;;
  *) echo "$MAJOR.$DATE.$REVISION" ;;
esac
