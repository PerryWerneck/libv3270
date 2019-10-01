#
# spec file for package libv3270
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

#---[ Package header ]------------------------------------------------------------------------------------------------

Summary:		3270 Virtual Terminal for GTK
Name:			libv3270
Version:		5.2
Release:		0
License:        LGPL-3.0
Source:			libv3270-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/libv3270
Group:			Development/Libraries/C and C++

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++
BuildRequires:	gettext-devel
BuildRequires:	m4

%if 0%{?fedora} ||  0%{?suse_version} > 1200

BuildRequires:	pkgconfig(lib3270)
BuildRequires:	pkgconfig(gtk+-3.0)

%else

BuildRequires:	lib3270-devel
BuildRequires:	gtk3-devel

%endif

%if 0%{?centos_version}
# CENTOS Genmarshal doesn't depends on python!
BuildRequires:	python
%endif

%description

Originally designed as part of the pw3270 application, this library
provides a TN3270 virtual terminal widget for GTK 3.

For more details, see https://softwarepublico.gov.br/social/pw3270/ .

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}-%{_libvrs}
Summary:		TN3270 access library
Group:			System/Libraries

%description -n %{name}-%{_libvrs}
Originally designed as part of the pw3270 application, this library
provides a TN3270 virtual terminal widget for GTK 3.

For more details, see https://softwarepublico.gov.br/social/pw3270/ .

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel

Summary:	Header files for the 3270 Virtual Terminal library
Group:		Development/Libraries/C and C++

Requires:	%{name}-%{_libvrs} = %{version}

%if 0%{?fedora} ||  0%{?suse_version} > 1200
Requires:	pkgconfig(lib3270)
%else
Requires:	lib3270-devel
%endif

%description devel
GTK development files for the 3270 Virtual Terminal.

%package -n glade-catalog-v3270

Summary:	Glade catalog for the TN3270 terminal emulator library
Group:		Development/Libraries/C and C++

Requires:	libv3270-devel = %{version}
Requires:	glade

%description -n glade-catalog-v3270
This package provides a catalog for Glade to allow the use of V3270
widgets in Glade.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup -n libv3270-%{version}

NOCONFIGURE=1 ./autogen.sh

%configure \
	--enable-pic

%build
make %{?_smp_mflags}

%install
%make_install
# configure --disable-static has no effect
rm -f %{buildroot}/%{_libdir}/*.a

%files -n %{name}-%{_libvrs}
%defattr(-,root,root)

# https://en.opensuse.org/openSUSE:Packaging_for_Leap#RPM_Distro_Version_Macros
%if 0%{?sle_version} > 120200
%doc AUTHORS README.md
%license LICENSE
%else
%doc AUTHORS README.md LICENSE
%endif

%{_libdir}/%{name}.so.%{MAJOR_VERSION}
%{_libdir}/%{name}.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files devel
%defattr(-,root,root)

%{_datadir}/pw3270/pot/*.pot

%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/*.pc

%{_includedir}/v3270.h
%{_includedir}/v3270

%files -n glade-catalog-v3270
%defattr(-,root,root)
%{_datadir}/glade/

%post -n %{name}-%{_libvrs} -p /sbin/ldconfig

%postun -n %{name}-%{_libvrs} -p /sbin/ldconfig

%changelog
