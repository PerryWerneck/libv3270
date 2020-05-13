#
# spec file for package libv3270
#
# Copyright (c) 2019 SUSE LLC
# Copyright (c) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


Name:           libv3270
Version:        5.2
Release:        0
Summary:        3270 Virtual Terminal for GTK
License:        LGPL-3.0-only
Group:          Development/Libraries/C and C++
URL:            https://github.com/PerryWerneck/libv3270
Source:         libv3270-%{version}.tar.xz
BuildRequires:  autoconf >= 2.61
BuildRequires:  automake
BuildRequires:  binutils
BuildRequires:  coreutils
BuildRequires:  gcc-c++
BuildRequires:  gettext-devel
BuildRequires:  m4
%if 0%{?fedora} ||  0%{?suse_version} > 1200
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  pkgconfig(lib3270) >= 5.2
%else
BuildRequires:  gtk3-devel
BuildRequires:  lib3270-devel >= 5.2
%endif
%if 0%{?centos_version} && 0%{?centos_version} < 800
# CENTOS Genmarshal doesn't depends on python!
BuildRequires:  python
%endif

%description

Originally designed as part of the pw3270 application, this library
provides a TN3270 virtual terminal widget for GTK 3.

For more details, see https://softwarepublico.gov.br/social/pw3270/ .

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define _product %(pkg-config --variable=product_name lib3270)
%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n %{name}-%{_libvrs}
Summary:        TN3270 Access library
Group:          Development/Libraries/C and C++

%description -n %{name}-%{_libvrs}

Originally designed as part of the pw3270 application this library provides a TN3270 virtual terminal widget for GTK 3.

See more details at https://softwarepublico.gov.br/social/pw3270/

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel
Summary:        Header files for the 3270 Virtual Terminal library
Group:          Development/Libraries/C and C++
Requires:       %{name}-%{_libvrs} = %{version}

%description devel
GTK development files for the 3270 Virtual Terminal.

%package -n glade-catalog-v3270
Summary:        Glade catalog for the TN3270 terminal emulator library
Group:          Development/Libraries/C and C++
Requires:       %{name}-%{_libvrs} = %{version}
Requires:       glade

%description -n glade-catalog-v3270
This package provides a catalog for Glade to allow the use of V3270

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup -q -n libv3270-%{version}

NOCONFIGURE=1 ./autogen.sh

%configure \
	--disable-static \
	--enable-pic \
	--with-release=%{release}

%build
make %{?_smp_mflags}

%install
%make_install
%find_lang libv3270 langfiles

%files -n %{name}-%{_libvrs} -f langfiles

# https://en.opensuse.org/openSUSE:Packaging_for_Leap#RPM_Distro_Version_Macros
%if 0%{?sle_version} > 120200
%doc AUTHORS README.md
%license LICENSE
%else
%doc LICENSE AUTHORS README.md
%endif

%{_libdir}/%{name}.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%{_datadir}/%{_product}/colors.conf

%dir %{_datadir}/%{_product}/remap/
%{_datadir}/%{_product}/remap/*.xml

%files devel

%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/*.pc

%{_includedir}/v3270.h
%{_includedir}/v3270

%{_datadir}/%{_product}/pot/*.pot

%files -n glade-catalog-v3270
%{_datadir}/glade/

%post -n %{name}-%{_libvrs} -p /sbin/ldconfig
%postun -n %{name}-%{_libvrs} -p /sbin/ldconfig

%changelog
