#  This file is part of StackAndConquer.
#  Copyright (C) 2015-present Thorsten Roth
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
#  along with StackAndConquer.  If not, see <https://www.gnu.org/licenses/>.

TEMPLATE      = app

unix: !macx {
       TARGET = stackandconquer
} else {
       TARGET = StackAndConquer
}

win32:VERSION  = 0.10.1.0
else:VERSION   = 0.10.1

QMAKE_TARGET_PRODUCT     = "StackAndConquer"
QMAKE_TARGET_DESCRIPTION = "Challenging tower conquest board game"
QMAKE_TARGET_COPYRIGHT   = "(C) 2015-present Thorsten Roth"

DEFINES      += APP_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\" \
                APP_VERSION=\"\\\"$$VERSION\\\"\" \
                APP_DESC=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\" \
                APP_COPY=\"\\\"$$QMAKE_TARGET_COPYRIGHT\\\"\"

MOC_DIR       = ./.moc
OBJECTS_DIR   = ./.objs
UI_DIR        = ./.ui
RCC_DIR       = ./.rcc

QT           += core gui qml widgets
lessThan(QT_MAJOR_VERSION, 6) {
  QT         += svg
} else {
  QT         += svgwidgets
}
CONFIG       += c++11
DEFINES      += QT_NO_FOREACH

CONFIG(debug, debug|release) {
  CONFIG     += warn_on
  DEFINES    += QT_DISABLE_DEPRECATED_BEFORE=0x060500
}

SOURCES      += main.cpp\
                stackandconquer.cpp \
                game.cpp \
                board.cpp \
                player.cpp \
                settings.cpp \
                opponentjs.cpp \
                generateboard.cpp

HEADERS      += stackandconquer.h \
                game.h \
                board.h \
                player.h \
                settings.h \
                opponentjs.h \
                generateboard.h

FORMS        += stackandconquer.ui \
                settings.ui

RESOURCES    += data/data.qrc \
                lang/translations.qrc

TRANSLATIONS += lang/stackandconquer_de.ts \
                lang/stackandconquer_en.ts \
                lang/stackandconquer_it.ts \
                lang/stackandconquer_jp.ts \
                lang/stackandconquer_nl.ts \
                lang/stackandconquer_pl.ts

win32:RC_ICONS = icons/stackandconquer.ico

macx {
  ICON               = icons/icon.icns
  QMAKE_INFO_PLIST   = data/mac/Info.plist

  CPU_DATA.path      = Contents/Resources
  CPU_DATA.files    += data/cpu
  CPU_DATA.files    += data/boards
  QMAKE_BUNDLE_DATA += CPU_DATA
}

unix: !macx {
  isEmpty(PREFIX) {
    PREFIX = /usr/local
  }
  isEmpty(BINDIR) {
    BINDIR = bin
  }

  target.path     = $$PREFIX/$$BINDIR/

  data.path       = $$PREFIX/share/stackandconquer
  data.files     += data/cpu
  data.files     += data/boards

  desktop.path    = $$PREFIX/share/applications
  desktop.files  += data/unix/com.github.elth0r0.stackandconquer.desktop

  icons.path      = $$PREFIX/share/icons
  icons.files    += icons/hicolor

  man.path        = $$PREFIX/share
  man.files      += man

  meta.path       = $$PREFIX/share/metainfo
  meta.files     += data/unix/com.github.elth0r0.stackandconquer.metainfo.xml

  INSTALLS       += target \
                    data \
                    desktop \
                    icons \
                    man \
                    meta
}
