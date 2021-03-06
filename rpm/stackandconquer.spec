#
# spec file for stackandconquer
#
# Copyright (C) 2015-2021 Thorsten Roth
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

Name:           stackandconquer
Summary:        Challenging tower conquest board game
Version:        0.9.0
Release:        1
License:        GPL-3.0+
URL:            https://github.com/ElTh0r0/stackandconquer
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
Group:          Amusements/Games

BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtsvg-devel
BuildRequires:  qt5-qtdeclarative-devel
%endif
#--------------------------------------------------------------------

# openSUSE or SLE
#--------------------------------------------------------------------
%if 0%{?suse_version}
Group:          Amusements/Games/Board/Other

BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  libqt5-qtbase-devel
BuildRequires:  libQt5Svg-devel
BuildRequires:  libqt5-qtdeclarative-devel
BuildRequires:  update-desktop-files
%endif
#--------------------------------------------------------------------

%description
StackAndConquer is a challenging tower conquest board game inspired
by "Mixtour". Objective is to build a stack of stones with at least
five stones and a stone with the players color on top.

%prep
%setup -q -n %{name}-%{version}

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
%build
# Create qmake cache file to add rpm optflags.
cat > .qmake.cache <<EOF
QMAKE_CXXFLAGS += %{optflags}
EOF
%{qmake_qt5} PREFIX=%{_prefix}
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
desktop-file-validate %{buildroot}/%{_datadir}/applications/com.github.elth0r0.stackandconquer.desktop
appstream-util validate-relax --nonet %{buildroot}/metainfo/com.github.elth0r0.stackandconquer.metainfo.xml

%clean
rm -rf %{buildroot}

%post
update-desktop-database &> /dev/null || :

%postun
update-desktop-database &> /dev/null || :
%endif
#--------------------------------------------------------------------

# openSUSE or SLE
#--------------------------------------------------------------------
%if 0%{?suse_version}
%build
# Create qmake cache file to add rpm optflags.
cat > .qmake.cache <<EOF
QMAKE_CXXFLAGS += %{optflags}
EOF
qmake-qt5 PREFIX=%{_prefix}
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%{buildroot} install
%suse_update_desktop_file com.github.elth0r0.stackandconquer

%clean
rm -rf %{buildroot}

%if 0%{?suse_version} >= 1140
%post
%desktop_database_post

%postun
%desktop_database_postun
%endif
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
%{_datadir}/applications/com.github.elth0r0.stackandconquer.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.*g
%{_datadir}/metainfo/com.github.elth0r0.stackandconquer.metainfo.xml
%doc COPYING
%{_mandir}/*/*

%changelog
