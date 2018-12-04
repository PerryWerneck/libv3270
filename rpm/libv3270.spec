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
License:        LGPL-3.0
Source:		%{name}-%{version}.tar.xz

Url:		https://portal.softwarepublico.gov.br/social/pw3270/

Group:		Development/Libraries/C and C++
BuildRoot:	/var/tmp/%{name}-%{version}

Provides:	libv3270_%{MAJOR_VERSION}_%{MINOR_VERSION}
Conflicts:	otherproviders(libv3270_%{MAJOR_VERSION}_%{MINOR_VERSION})

BuildRequires:  pkgconfig(openssl)
BuildRequires:	pkgconfig(lib3270)
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  autoconf >= 2.61
BuildRequires:  automake
BuildRequires:  binutils
BuildRequires:  coreutils
BuildRequires:  gcc-c++
BuildRequires:  gettext-devel
BuildRequires:  m4

%description

TN3270 GTK Virtual terminal Widget originally designed as part of the pw3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel

Summary:	3270 Virtual Terminal for GTK development files
Group:		Development/Libraries/C and C++

Requires:	%{name} = %{version}

Provides:	libv3270-devel = %{version}
Conflicts:	otherproviders(libv3270-devel)

%description devel

3270 Virtual Terminal for GTK development files.

Originally designed as part of the pw3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

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

%{_libdir}/libv3270.so.5
%{_libdir}/libv3270.so.5.2

%files devel
%defattr(-,root,root)

%{_libdir}/libv3270.so
%{_libdir}/pkgconfig/libv3270.pc

%{_includedir}/libv3270.h
%{_includedir}/libv3270

%{_libdir}/libv3270.a

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
