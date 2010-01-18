#!/bin/sh

  
MAJOR="0"
DATE="$(date +%Y%m%d)"
#REVISION=$([ -d .svn ] && svn info . | awk '/Revision:/ {print $2}')
REVISION=00


VERSION="$MAJOR.$DATE.$REVISION"

case $1 in
  --major) echo "$MAJOR" ;;
  --header) 
    echo "#ifndef _VERSION_H_"
    echo "#define _VERSION_H_"
    echo "static const char *XDO_VERSION = \"$VERSION\";"
    echo "#endif /* ifndef _VERSION_H */"
    ;;
  *) echo "$VERSION" ;;
esac
