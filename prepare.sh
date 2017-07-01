#!/bin/sh

set -e

RFOLDER="`pwd`"

echo "***Prepare test build***"

mv $RFOLDER/debian/changelog $RFOLDER/debian/changelog.template
DISTRIB=$(lsb_release -c -s)
sed "s/unstable/$DISTRIB/g" $RFOLDER/debian/changelog.template > $RFOLDER/debian/changelog	

if [ ! $ARCHITECTURE ]; then
		ARCHITECTURE=64
fi

if [ ! $TMPBUILDDIR ]; then
		TMPBUILDDIR=build/release
fi

qmake ./tcpview.pro -r -spec linux-g++-$ARCHITECTURE MOC_DIR=$TMPBUILDDIR RCC_DIR=$TMPBUILDDIR UI_DIR=$TMPBUILDDIR OBJECTS_DIR=$TMPBUILDDIR
# make

echo ">>>ARCHITECTURE\t=$ARCHITECTURE"
echo ">>>TMPBUILDDIR\t=$TMPBUILDDIR"
echo ">>>DISTRIB\t=$DISTRIB"
echo ">>>RFOLDER\t=$RFOLDER"

