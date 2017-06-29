#!/bin/sh

set -e

RFOLDER="`pwd`"

mv ${RFOLDER}/debian/changelog ${RFOLDER}/debian/changelog.template
DISTRIB=$(lsb_release -c -s)
sed "s/unstable/${DISTRIB}/g" ${RFOLDER}/debian/changelog.template > ${RFOLDER}/debian/changelog	

qmake ./tcpview.pro -r -spec linux-g++-64 MOC_DIR=build/release RCC_DIR=build/release UI_DIR=build/release OBJECTS_DIR=build/release
# make

echo "DISTRIB=${DISTRIB}"
echo "RFOLDER=${RFOLDER}"

