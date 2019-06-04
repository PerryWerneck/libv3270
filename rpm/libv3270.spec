#
# spec file for packages libv3270
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

#---[ Versions ]------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION 5
%define MINOR_VERSION 2

%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

#Compat macro for new _fillupdir macro introduced in Nov 2017
%if ! %{defined _fillupdir}
  %define _fillupdir /var/adm/fillup-templates
%endif

#---[ Macros ]--------------------------------------------------------------------------------------------------------

%if ! %{defined _release}
  %define _release suse%{suse_version}
%endif

#---[ Main package ]--------------------------------------------------------------------------------------------------

Summary:	3270 Virtual Terminal for GTK
Name:		libv3270-%{_libvrs}
Version:	5.2
Release:	0
License:	LGPL-3.0
Source:		libv3270-%{version}.tar.xz

Url:		https://github.com/PerryWerneck/libv3270.git

Group:		Development/Libraries/C and C++
BuildRoot:	/var/tmp/%{name}-%{version}

BuildRequires:	lib3270-devel
BuildRequires:  autoconf >= 2.61
BuildRequires:  automake
BuildRequires:  binutils
BuildRequires:  coreutils
BuildRequires:  gcc-c++
BuildRequires:  gettext-devel
BuildRequires:  m4

%if 0%{?fedora} ||  0%{?suse_version} > 1200

BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(gtk+-3.0)

%else

BuildRequires:  openssl-devel
BuildRequires:	gtk3-devel

%endif

# https://en.opensuse.org/openSUSE:Build_Service_cross_distribution_howto
%if %{undefined fedora} && %{undefined rhel_version} && %{undefined centos_version}
BuildRequires:	libgladeui-2-6
%endif

%if 0%{?centos_version}
# centos requires python for genmarshal
BuildRequires:  python
%endif

%description

Originally designed as part of the pw3270 application this library provides a TN3270 virtual terminal widget for GTK 3.

See more details at https://softwarepublico.gov.br/social/pw3270/

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n libv3270-devel

Summary:	3270 Virtual Terminal for GTK development files
Group:		Development/Libraries/C and C++

Requires:	%{name} = %{version}
Requires:	lib3270-devel

%description -n libv3270-devel

3270 Virtual Terminal for GTK development files.

Originally designed as part of the pw3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

%package -n glade-catalog-v3270

Summary:	Glade catalog for the TN3270 terminal emulator library
Group:		Development/Libraries/C and C++

Requires:	libv3270-devel = %{version}
Requires:	glade

%description -n glade-catalog-v3270

3270 Virtual Terminal for GTK development files.

Originally designed as part of the pw3270 application.

This package provides a catalog for Glade, to allow the use of V3270
widgets in Glade.

See more details at https://softwarepublico.gov.br/social/pw3270/


#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup -n libv3270-%{version}

NOCONFIGURE=1 ./autogen.sh

%configure \
	--with-sdk-version=%{version}

%build
make clean
make all

%install
rm -rf $RPM_BUILD_ROOT

%makeinstall

%files
%defattr(-,root,root)
%doc AUTHORS LICENSE README.md

%{_libdir}/libv3270.so.%{MAJOR_VERSION}
%{_libdir}/libv3270.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n libv3270-devel
%defattr(-,root,root)

%{_libdir}/libv3270.so
%{_libdir}/pkgconfig/*.pc

%{_includedir}/v3270.h
%{_includedir}/v3270

%{_libdir}/*.a
%{_datadir}/pw3270/pot/*.pot

%files -n glade-catalog-v3270
%defattr(-,root,root)
/usr/share/glade/catalogs/v3270.xml
/usr/share/glade/pixmaps/hicolor/16x16/actions/widget-v3270-terminal.png
/usr/share/glade/pixmaps/hicolor/22x22/actions/widget-v3270-terminal.png

%pre
/sbin/ldconfig
exit 0

%post
/sbin/ldconfig
exit 0

%postun
/sbin/ldconfig
exit 0

%changelog
