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

TEMPLATE      = lib
CONFIG       += plugin
TARGET        = cpudummy
DESTDIR       = ../

VERSION       = 0.1.0
QMAKE_TARGET_DESCRIPTION = "Dummy CPU opponent for StackAndConquer"
QMAKE_TARGET_COPYRIGHT   = "(C) 2016 Thorsten Roth"

DEFINES      += PLUGIN_NAME=\\\"$$TARGET\\\" \
                PLUGIN_VERSION=\"\\\"$$VERSION\\\"\"

MOC_DIR       = ./.moc
OBJECTS_DIR   = ./.objs
UI_DIR        = ./.ui
RCC_DIR       = ./.rcc

HEADERS       = CDummy.h

SOURCES       = CDummy.cpp
