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

TEMPLATE       = subdirs
CONFIG        += ordered
SUBDIRS        = cpu/dummy \
                 application

unix: !macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }

    desktop.path    = $$PREFIX/share/applications
    desktop.files  += stackandconquer.desktop

    man.path        = $$PREFIX/share
    man.files      += man

    INSTALLS       += desktop \
                      man
}
