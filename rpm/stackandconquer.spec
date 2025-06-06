#
# spec file for stackandconquer
#
# Copyright (C) 2015-present Thorsten Roth
#
# StackAndConquer is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# StackAndConquer is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with StackAndConquer  If not, see <http://www.gnu.org/licenses/>.

%define  _name  com.github.elth0r0.stackandconquer
Name:           stackandconquer
Summary:        Challenging tower conquest board game
Version:        0.11.1
Release:        1
License:        GPL-3.0-or-later
Group:          Amusements/Games/Board/Other
URL:            https://github.com/ElTh0r0/stackandconquer
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

#--------------------------------------------------------------------

BuildRequires:  cmake
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6LinguistTools)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6SvgWidgets)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  update-desktop-files

#--------------------------------------------------------------------

%description
StackAndConquer is a challenging tower conquest board game inspired by
Mixtour created by Dieter Stein. Objective is to build a stack of stones
with at least five stones and a stone with the players color on top.

%prep
%autosetup -N -n %{name}-%{version}

%build
%cmake_qt6
%qt6_build

%install
%qt6_install

%files
%doc README.md
%{_bindir}/%{name}
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/cpu/
%{_datadir}/%{name}/boards/
%{_datadir}/applications/%{_name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{_name}.??g
%{_mandir}/man?/%{name}.?%{?ext_man}
%{_mandir}/??/man?/%{name}.?%{?ext_man}
%{_datadir}/metainfo/%{_name}.metainfo.xml
%license COPYING

%changelog
