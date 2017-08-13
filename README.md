# tcpview

Graphical network connections viewer for Linux

To get the last release version, please use the PPA:

Artful, Zesty 
https://launchpad.net/~chipmunk.sm/+archive/ubuntu/ppa

Home page:

https://github.com/chipmunk-sm/tcpview

![screenshot_201708111](https://user-images.githubusercontent.com/29524958/29194415-231f2b9e-7e32-11e7-8c94-8eac6ea0bf98.png)


# How to build

## Install Qt

```sh
$ sudo apt-get install build-essential
$ sudo apt-get install qtcreator
$ sudo apt-get install qt5-default
```

## Clone
Open terminal in your preferred folder:

```sh
$ git clone https://github.com/chipmunk-sm/tcpview.git
```

## Build

Either open `tcpview.pro` with Qt Creator for editing and building, or from the terminal:

```sh
$ cd tcpview
$ qmake
$ make
```

## Run tcpview

```sh
$ ./tcpview
```

# Translation

Use the Qt Linguist tool to translate text

```
$ sudo apt-get install qttools5-dev-tools
```

Open tcpview folder with terminal and update translations:

```sh
$ lupdate -no-obsolete -verbose -pro tcpview.pro
```

Open all editable translations:

```sh
$ cd translations
$ linguist language_en.ts language_cs.ts language_de.ts language_fr.ts language_ja.ts language_pl.ts language_ru.ts language_sl.ts language_zh_CN.ts language_zh_TW.ts
$ cd ..
```

And update `.qm` files:

```sh
$ lrelease -removeidentical -compress tcpview.pro
```

Finally, [rebuild tcpview](#how-to-build).

## Adding new translations

Add a new translations `xx.ts` file to tcpview.pro, rebuild tcpview and add your new `xx.qm` to `tcpview.qrc`. Finally [rebuild tcpview](#how-to-build).
