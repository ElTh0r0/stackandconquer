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

%define  _name  org.codeberg.elth0r0.stackandconquer
Name:           stackandconquer
Summary:        Challenging tower conquest board game
Version:        0.11.1
Release:        1
License:        GPL-3.0-or-later
URL:            https://codeberg.org/ElTh0r0/stackandconquer
Source:         %{name}-v%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
Group:          Amusements/Games
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  ninja-build
%endif
#--------------------------------------------------------------------
# openSUSE
#--------------------------------------------------------------------
%if 0%{?suse_version}
Group:          Amusements/Games/Board/Other
%endif
#--------------------------------------------------------------------
# All
#--------------------------------------------------------------------
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  cmake
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6SvgWidgets)
BuildRequires:  cmake(Qt6LinguistTools)
#--------------------------------------------------------------------

%description
StackAndConquer is a challenging tower conquest board game inspired by
Mixtour created by Dieter Stein. Objective is to build a stack of stones
with at least five stones and a stone with the players color on top.

%prep
%autosetup -n %{name} -p1

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
%build
%cmake_qt6
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{_name}.desktop || :
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/%{_name}.metainfo.xml || :
%endif
#--------------------------------------------------------------------
# openSUSE
#--------------------------------------------------------------------
%if 0%{?suse_version}
%build
%cmake_qt6
%{qt6_build}

%install
%{qt6_install}
%endif
#--------------------------------------------------------------------

%files
%defattr(-,root,root,-)
%if 0%{?suse_version}
%dir %{_datadir}/metainfo
%{_datadir}/icons/hicolor/
%endif
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{_name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{_name}.*g
%{_datadir}/metainfo/%{_name}.metainfo.xml
%doc COPYING
%{_mandir}/*/*

%changelog
