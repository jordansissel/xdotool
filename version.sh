#!/bin/sh

if [ -r "VERSION" ] ; then
  . ./VERSION
fi

if [ -z "$MAJOR" -o -z "$RELEASE" -o -z "$REVISION" ] ; then
  MAJOR="3"
  SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(date +%s)}"
  DATE_FMT="+%Y%m%d"
  RELEASE="$(date -u -d "@$SOURCE_DATE_EPOCH" "+$DATE_FMT" 2>/dev/null || date -u -r "$SOURCE_DATE_EPOCH" "+$DATE_FMT" 2>/dev/null || date -u "+$DATE_FMT")"
  REVISION=1
  #$([ -d .svn ] && svn info . | awk '/Revision:/ {print $2}')
  : ${REVISION=:0}
fi

VERSION="$MAJOR.$RELEASE.$REVISION"

case $1 in
  --major) echo "$MAJOR" ;;
  --header) 
    echo "#ifndef _VERSION_H_"
    echo "#define _VERSION_H_"
    echo "static const char *XDO_VERSION = \"$VERSION\";"
    echo "#endif /* ifndef _VERSION_H */"
    ;;
  --shell)
    echo "MAJOR=\"$MAJOR\""
    echo "RELEASE=\"$RELEASE\""
    echo "REVISION=\"$REVISION\""
    ;;
  *) echo "$VERSION" ;;
esac
