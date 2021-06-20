#/* This file is part of "TcpView For Linux" - network connections viewer for Linux
# * Copyright (C) 2021 chipmunk-sm <dannico@linuxmail.org>
# *
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program.  If not, see <http://www.gnu.org/licenses/>.
# */

QT += core gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -luuid

TARGET = tcpview
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    source/buffer.cpp \
    source/configdialog.cpp \
    source/connectionstatehelper.cpp \
    source/cstylehelperex.cpp \
    source/datasource.cpp \
    source/rootmodule.cpp \
    source/cconnectionstree.cpp \
    source/ccustomproxymodel.cpp \
    source/ccfontsize.cpp \
    source/cfilesavewrapper.cpp \
    source/cportservicenames.cpp \
    source/cusername.cpp \
    source/clanguage.cpp

HEADERS  += mainwindow.h \
    source/buffer.h \
    source/configdialog.h \
    source/connectionstatehelper.h \
    source/cstylehelperex.h \
    source/datasource.h \
    source/rootmodule.h \
    source/cconnectionstree.h \
    source/tableheadercaption.h \
    source/ccustomproxymodel.h \
    source/ccfontsize.h \
    source/cfilesavewrapper.h \
    source/cportservicenames.h \
    source/cusername.h \
    source/clanguage.h \
    source/defined.h

FORMS    += mainwindow.ui

DISTFILES += \
    data/tcpview.svg \
    data/tcpview.desktop \
    debian/compat \
    debian/control \
    debian/copyright \
    debian/tcpview.install \
    debian/rules \
    debian/source/format \
    README.md \
    LICENSE \
    debian/changelog

RESOURCES += \
    tcpview.qrc

# $HOME/Qt/5.15.2/gcc_64/bin/lupdate -no-obsolete -verbose -pro tcpview.pro
# lupdate -no-obsolete -verbose -pro tcpview.pro

# cd translations

# $HOME/Qt/5.15.2/gcc_64/bin/linguist language_en.ts language_cs.ts language_de.ts language_es.ts language_fr.ts language_ja.ts language_pl.ts language_ru.ts language_sl.ts language_zh_CN.ts language_zh_TW.ts
# language_en.ts language_cs.ts language_de.ts language_es.ts language_fr.ts language_ja.ts language_pl.ts language_ru.ts language_sl.ts language_zh_CN.ts language_zh_TW.ts

# cd ..

# $HOME/Qt/5.15.2/gcc_64/bin/lrelease -removeidentical -compress tcpview.pro
# lrelease -removeidentical -compress tcpview.pro

TRANSLATIONS += \
    translations/language_en.ts \
    translations/language_cs.ts \
    translations/language_de.ts \
    translations/language_es.ts \
    translations/language_fr.ts \
    translations/language_ja.ts \
    translations/language_pl.ts \
    translations/language_ru.ts \
    translations/language_sl.ts \
    translations/language_zh_CN.ts \
    translations/language_zh_TW.ts

#af		Afrikaans
#sq		Albanian
#ar		Arabic
#eu		Basque
#be		Belarusian
#bs		Bosnian
#bg		Bulgarian
#ca		Catalan
#hr		Croatian
#zh_cn	Chinese (Simplified)
#zh_tw	Chinese (Traditional)
#cs		Czech
#da		Danish
#nl		Dutch
#en		English
#en_us	English (US)
#et		Estonian
#fa		Farsi
#fil	Filipino
#fi		Finnish
#fr		French
#fr_ca	French (Canada)
#ga		Gaelic
#gl		Gallego
#ka		Georgian
#de		German
#de_du	German (Personal)
#el		Greek
#gu		Gujarati
#he		Hebrew
#hi		Hindi
#hu		Hungarian
#is		Icelandic
#id		Indonesian
#it		Italian
#ja		Japanese
#kn		Kannada
#km		Khmer
#ko		Korean
#lo		Lao
#lt		Lithuanian
#lv		Latvian
#ml		Malayalam
#ms		Malaysian
#mi_tn	Maori (Ngai Tahu)
#mi_wwow	Maori (Waikoto Uni)
#mn		Mongolian
#no		Norwegian
#no_gr	Norwegian (Primary)
#nn		Nynorsk
#pl		Polish
#pt		Portuguese
#pt_br	Portuguese (Brazil)
#ro		Romanian
#ru		Russian
#sm		Samoan
#sr		Serbian
#sk		Slovak
#sl		Slovenian
#so		Somali
#es		Spanish (International)
#sv		Swedish
#tl		Tagalog
#ta		Tamil
#th		Thai
#to		Tongan
#tr		Turkish
#uk		Ukrainian
#vi		Vietnamese
