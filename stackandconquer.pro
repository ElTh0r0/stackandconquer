# SPDX-FileCopyrightText: 2015-2025 Thorsten Roth
# SPDX-License-Identifier: GPL-3.0-or-later

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {
  error("StackAndConquer requires Qt 5.12 or greater")
}

TEMPLATE      = app

unix: !macx {
       TARGET = stackandconquer
} else {
       TARGET = StackAndConquer
}

win32:VERSION  = 0.12.0.0
else:VERSION   = 0.12.0

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
  QMAKE_CXXFLAGS += -Wall -Wextra -pedantic
  DEFINES    += QT_DISABLE_DEPRECATED_BEFORE=0x061000
}

SOURCES      += main.cpp\
                stackandconquer.cpp \
                game.cpp \
                board.cpp \
                player.cpp \
                settings.cpp \
                settingsdialog.cpp \
                opponentjs.cpp \
                generateboard.cpp

HEADERS      += stackandconquer.h \
                game.h \
                board.h \
                player.h \
                settings.h \
                settingsdialog.h \
                opponentjs.h \
                generateboard.h

FORMS        += stackandconquer.ui \
                settingsdialog.ui

RESOURCES    += data/data.qrc \
                lang/translations.qrc

TRANSLATIONS += lang/stackandconquer_de.ts \
                lang/stackandconquer_en.ts \
                lang/stackandconquer_it.ts \
                lang/stackandconquer_ja_JP.ts \
                lang/stackandconquer_nl.ts \
                lang/stackandconquer_pl.ts \
                lang/stackandconquer_ru.ts

win32:RC_FILE = data/win.rc

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

  target.path    = $$PREFIX/$$BINDIR/

  data.path      = $$PREFIX/share/stackandconquer
  data.files    += data/cpu
  data.files    += data/boards

  desktop.path   = $$PREFIX/share/applications
  desktop.files += data/unix/com.github.elth0r0.stackandconquer.desktop

  icons.path     = $$PREFIX/share/icons
  icons.files   += icons/hicolor

  man.path       = $$PREFIX/share/man
  # Specify each subfolder - otherwise CMakeLists.txt will be installed
  man.files     += man/man6
  man.files     += man/de
  man.files     += man/it
  man.files     += man/ru

  meta.path      = $$PREFIX/share/metainfo
  meta.files    += data/unix/com.github.elth0r0.stackandconquer.metainfo.xml

  INSTALLS      += target \
                   data \
                   desktop \
                   icons \
                   man \
                   meta
}
