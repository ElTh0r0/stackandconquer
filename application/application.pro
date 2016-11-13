#  This file is part of StackAndConquer.
#  Copyright (C) 2015-2016 Thorsten Roth
#
#  StackAndConquer is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  StackAndConquer is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with StackAndConquer.  If not, see <http://www.gnu.org/licenses/>.

TEMPLATE      = app

unix: !macx {
       TARGET = stackandconquer
} else {
       TARGET = StackAndConquer
}

DESTDIR       = ../

VERSION       = 0.5.0
QMAKE_TARGET_PRODUCT     = "StackAndConquer"
QMAKE_TARGET_DESCRIPTION = "Challenging tower conquest board game"
QMAKE_TARGET_COPYRIGHT   = "(C) 2015-2016 Thorsten Roth"

DEFINES      += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                APP_VERSION=\"\\\"$$VERSION\\\"\" \
                APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR       = ./.moc
OBJECTS_DIR   = ./.objs
UI_DIR        = ./.ui
RCC_DIR       = ./.rcc

QT           += core gui svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES      += main.cpp\
                CStackAndConquer.cpp \
                CGame.cpp \
                CBoard.cpp \
                CPlayer.cpp \
                CSettings.cpp \
                CCpuOpponents.cpp

HEADERS      += CStackAndConquer.h \
                CGame.h \
                CBoard.h \
                CPlayer.h \
                CSettings.h \
                CCpuOpponents.h \
                ICpuOpponent.h

FORMS        += CStackAndConquer.ui \
                CSettings.ui

RESOURCES    += res/stackandconquer_resources.qrc
win32:RC_FILE = res/stackandconquer.rc

TRANSLATIONS += lang/stackandconquer_de.ts

unix: !macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = bin
    }

    target.path     = $$PREFIX/$$BINDIR/

    #data.path       = $$PREFIX/share/stackandconquer
    #data.files     += data

    lang.path       = $$PREFIX/share/stackandconquer/lang
    lang.files     += lang/*.qm

    #icons.path      = $$PREFIX/share/icons
    #icons.files    += res/images/hicolor
    
    pixmap.path     = $$PREFIX/share/pixmaps
    pixmap.files   += res/images/stackandconquer_64x64.png \
                      res/images/stackandconquer.xpm

    INSTALLS       += target \
                      #data \
                      lang \
                      #icons \
                      pixmap
}