# tcpview

Graphical network connections viewer for Linux

To get the last release version, please use the PPA:

Artful, Zesty 
https://launchpad.net/~chipmunk.sm/+archive/ubuntu/ppa

Home page:

https://github.com/chipmunk-sm/tcpview

![screenshot_201708111](https://user-images.githubusercontent.com/29524958/29194415-231f2b9e-7e32-11e7-8c94-8eac6ea0bf98.png)


# How to build

# Install Qt

$sudo apt-get install build-essential

$sudo apt-get install qtcreator

$sudo apt-get install qt5-default

# Clone sourcess with HTTPS. 
Open terminal in your preferred folder

$git clone https://github.com/chipmunk-sm/tcpview.git

# Open tcpview.pro with Qt Creator for edit/build 

or build from terminal

$cd tcpview

$qmake

$make

# Run tcpview

$./tcpview

# Use the Qt Linguist tool to translate text

$sudo apt-get install qttools5-dev-tools

open tcpview folder with terminal

update translations

$lupdate -no-obsolete -verbose -pro tcpview.pro

open all available translations for edit

$cd translations

$linguist language_en.ts language_cs.ts language_de.ts language_fr.ts language_ja.ts language_pl.ts language_ru.ts language_sl.ts language_zh_CN.ts language_zh_TW.ts

$cd ..

update .qm files

$lrelease -removeidentical -compress tcpview.pro

rebuild tcpview

# Add new translations

*add new translations xx.ts (just write name) to tcpview.pro, rebuild tcpview and add new xx.qm to tcpview.qrc, rebuild tcpview*
