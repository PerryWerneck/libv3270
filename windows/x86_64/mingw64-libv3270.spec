#
# spec file for package mingw64-libv3270
#
# Copyright (c) 2014 SUSE LINUX Products GmbH, Nuernberg, Germany.
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

#---[ Main package ]--------------------------------------------------------------------------------------------------

Summary:		3270 Virtual Terminal for GTK
Name:			mingw64-libv3270
Version: 5.5.0

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}
%define _product %(x86_64-w64-mingw32-pkg-config --variable=product_name lib3270)

Release:		0
License:		GPL-2.0

Source:			libv3270-%{version}.tar.xz

Url:			https://github.com/PerryWerneck/libv3270.git

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

Provides:		mingw64(lib:v3270)

Requires:		mingw64(libgdk_pixbuf-2.0-0.dll)
BuildRequires:	mingw64(pkg:gdk-pixbuf-2.0)

BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	gettext-tools
BuildRequires:  pkgconfig(glib-2.0)

BuildRequires:	mingw64-cross-binutils
BuildRequires:	mingw64-cross-gcc
BuildRequires:	mingw64-cross-gcc-c++
BuildRequires:	mingw64-cross-pkg-config
BuildRequires:	mingw64-filesystem

BuildRequires:	mingw64(pkg:gtk+-win32-3.0)
BuildRequires:	mingw64(lib:iconv)
BuildRequires:	mingw64(lib:intl)

BuildRequires:	mingw64(pkg:lib3270)

%_mingw64_package_header_debug

BuildArch: noarch

%description

TN3270 GTK Virtual terminal Widget originally designed as part of the pw3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel

Summary:	3270 Virtual Terminal for GTK development files
Group:		Development/Libraries/C and C++

Requires:	%{name} = %{version}
Requires:	mingw64(pkg:lib3270)

%description devel

3270 Virtual Terminal for GTK development files.

Originally designed as part of the pw3270 application.

See more details at https://softwarepublico.gov.br/social/pw3270/

%package -n mingw64-glade-catalog-v3270

Summary:	Glade catalog for the TN3270 terminal emulator library
Group:		Development/Libraries/C and C++

Requires:	%{name}-devel = %{version}
Requires:	glade

%description -n mingw64-glade-catalog-v3270

3270 Virtual Terminal for GTK development files.

Originally designed as part of the pw3270 application.

This package provides a catalog for Glade, to allow the use of V3270
widgets in Glade.

See more details at https://softwarepublico.gov.br/social/pw3270/

%_mingw64_debug_package

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup -n libv3270-%{version}

NOCONFIGURE=1 ./autogen.sh

%{_mingw64_configure} \
	--with-sdk-version=%{version}

%build
make clean
make all

%{_mingw64_strip} \
	--strip-all \
    .bin/Release/*.dll

%install
%{_mingw64_makeinstall}
%_mingw64_find_lang libv3270-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%clean
rm -rf %{buildroot}

#---[ Files ]---------------------------------------------------------------------------------------------------------

%files -f langfiles
%defattr(-,root,root)
%doc AUTHORS LICENSE README.md

%{_mingw64_bindir}/*.dll
%{_mingw64_datadir}/%{_product}/colors.conf
%dir %{_mingw64_datadir}/%{_product}/remap
%{_mingw64_datadir}/%{_product}/remap/*.xml

%exclude %{_mingw64_datadir}/*.changes

%files devel
%defattr(-,root,root)
%{_mingw64_includedir}/v3270
%{_mingw64_includedir}/v3270.h

%{_mingw64_libdir}/pkgconfig/*.pc
%{_mingw64_libdir}/*.a

%{_mingw64_datadir}/%{_product}/pot/*.pot

%{_mingw64_datadir}/%{_product}/def/*.def

%files -n mingw64-glade-catalog-v3270
%defattr(-,root,root)

%dir %{_mingw64_datadir}/glade
%dir %{_mingw64_datadir}/glade/catalogs
%{_mingw64_datadir}/glade/catalogs/v3270.xml

%dir %{_mingw64_datadir}/glade/pixmaps
%dir %{_mingw64_datadir}/glade/pixmaps/hicolor
%dir %{_mingw64_datadir}/glade/pixmaps/hicolor/16x16
%dir %{_mingw64_datadir}/glade/pixmaps/hicolor/22x22
%dir %{_mingw64_datadir}/glade/pixmaps/hicolor/16x16/actions
%dir %{_mingw64_datadir}/glade/pixmaps/hicolor/22x22/actions
%{_mingw64_datadir}/glade/pixmaps/hicolor/16x16/actions/widget-v3270-terminal.png
%{_mingw64_datadir}/glade/pixmaps/hicolor/22x22/actions/widget-v3270-terminal.png

%changelog

