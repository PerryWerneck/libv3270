dnl SPDX-License-Identifier: LGPL-3.0-or-later

dnl Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
dnl
dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU Lesser General Public License as published
dnl by the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with this program.  If not, see <https://www.gnu.org/licenses/>.

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([libv3270], [5.5], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Recommended for gtk-doc (https://developer.gnome.org/gtk-doc-manual/stable/settingup_autoconf.html.en)
AC_CONFIG_MACRO_DIR(m4)

dnl Initialize libtool.
LT_INIT

dnl Compute the canonical host-system type
AC_CANONICAL_HOST

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

dnl Check for iconv
AM_ICONV

dnl Set gettext version
AM_GNU_GETTEXT_VERSION([0.14])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_SED
AC_PROG_LN_S

AC_LANG([C])

dnl ---------------------------------------------------------------------------
dnl Initialize defaults
dnl ---------------------------------------------------------------------------

DBG_CFLAGS="-g -fstack-check -DDEBUG=1"
RLS_CFLAGS="-DNDEBUG=1"
PLUGINS=""
APP_RESOURCES=""
APP_LDFLAGS=""
DLL_LDFLAGS="-shared -Wl,-soname,\$(@F)"
STATIC_LDFLAGS=""
INSTALL_PACKAGES="dev glade"

dnl ---------------------------------------------------------------------------
dnl Check for OS specifics
dnl ---------------------------------------------------------------------------

DLLPREFIX="lib"

case "$host" in
	*-mingw32|*-pc-msys)
		app_cv_osname="windows"
		CFLAGS="$CFLAGS -pthread -D_WIN32_WINNT=0x0600"
		LIBS="$LIBS -lws2_32 -lwtsapi32 -lcomdlg32"
		LDFLAGS="$LDFLAGS -pthread"
		DLL_LDFLAGS="-shared -Wl,--output-def,\$(@D)/\$(LIBNAME).def"
		DLLEXT=".dll"

		INSTALL_PACKAGES="windows-lib ${INSTALL_PACKAGES}"

		app_cv_static='yes'

		app_win32_revision=$(date +%-y.%-m.%-d.%-H)
		AC_SUBST(WIN32_VERSION,$app_win32_revision)

		app_win32_file_version=$(date +%-y,%-m,%-d,%-H)
		AC_SUBST(WIN32_FILE_VERSION,$app_win32_file_version)

		AC_CONFIG_FILES(src/terminal/windows/resources.rc)
		;;

	*-apple-darwin*)
		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"
		app_cv_osname="macos"
		LOGDIR="/var/log"
		DLLEXT=".dylib"
		DLL_LDFLAGS="-shared -Wl,-install_name,\$(@F)"

		INSTALL_PACKAGES="macos-lib ${INSTALL_PACKAGES}"

		app_cv_static='no'
		;;

 	*)
		CFLAGS="$CFLAGS -pthread"
		LDFLAGS="$LDFLAGS -pthread"
		app_cv_osname="linux"
		LOGDIR="/var/log"
		DLLEXT=".so"

		INSTALL_PACKAGES="linux-lib ${INSTALL_PACKAGES}"

		app_cv_static='no'

		AC_CONFIG_FILES(debian/control)
esac

AC_SUBST(OSNAME,$app_cv_osname)
AC_SUBST(LIBS)
AC_SUBST(LOGDIR)
AC_SUBST(DLLEXT)
AC_SUBST(DLLPREFIX)
AC_SUBST(DLL_LDFLAGS)

dnl ---------------------------------------------------------------------------
dnl Check for other programs
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([AR], [ar], [ar])
AC_PATH_TOOL([ZIP],[zip],[no])
AC_PATH_TOOL([DLLTOOL],[dlltool],[no])
AC_PATH_TOOL([WINDRES], [windres], [no])

PKG_CHECK_EXISTS

dnl ---------------------------------------------------------------------------
dnl Version info
dnl ---------------------------------------------------------------------------

AC_SUBST(PACKAGE_DESCRIPTION,"3270 Virtual Terminal for GTK")

app_vrs_major=$(echo $VERSION | cut -d. -f1)
app_vrs_minor=$(echo $VERSION | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major,[ The package major version ])
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor,[ The package minor version ])

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"libv3270-$app_vrs_major.$app_vrs_minor",[The gettext package name.])
AC_SUBST(GETTEXT_PACKAGE,"libv3270-$app_vrs_major.$app_vrs_minor")

case "$host" in
	*-mingw32|*-pc-msys)
		AC_SUBST(SONAME,libv3270.dll)
		;;

	*-apple-darwin*)
		AC_SUBST(SONAME,libv3270-$app_vrs_major.$app_vrs_minor.dylib)
		;;

 	*)
		AC_SUBST(SONAME,libv3270.so.$app_vrs_major.$app_vrs_minor)

esac

dnl ---------------------------------------------------------------------------
dnl Check for libm (Required for spinner)
dnl ---------------------------------------------------------------------------

AC_SEARCH_LIBS( [sin], [m], AC_DEFINE(HAVE_LIBM,1,[ Have libm ]), AC_MSG_ERROR([libm not present.]))

dnl ---------------------------------------------------------------------------
dnl Check for libintl
dnl ---------------------------------------------------------------------------

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([GENMARSHAL], [glib-genmarshal], [glib-genmarshal])
AC_PATH_TOOL([STRIP], [strip], [true])

dnl ---------------------------------------------------------------------------
dnl Check for LIB3270
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIB3270], [lib3270], AC_DEFINE(HAVE_LIB3270,1,[ The TN3270 protocol library ]), AC_MSG_ERROR([LIB3270 not present.]))

AC_SUBST(LIB3270_LIBS)
AC_SUBST(LIB3270_CFLAGS)

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=`pkg-config --variable=product_name lib3270` ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, $app_cv_product,[The product name])
AC_SUBST(PRODUCT_NAME,$app_cv_product)

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GTK], [gtk4], AC_DEFINE(HAVE_GTK,1,[ The GTK Library ]), AC_MSG_ERROR([GTK4 not present.]))

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_MODVERSION,`$PKG_CONFIG --modversion gtk4`)

dnl ---------------------------------------------------------------------------
dnl Check for GTK DOC
dnl ---------------------------------------------------------------------------

dnl https://developer.gnome.org/gtk-doc-manual/stable/settingup_autoconf.html.en
m4_ifdef([GTK_DOC_CHECK], [
GTK_DOC_CHECK([1.14],[--flavour no-tmpl])
],[
AM_CONDITIONAL([ENABLE_GTK_DOC], false)
])

dnl ---------------------------------------------------------------------------
dnl Check for pic
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--disable-pic], [disable support for PIC libraries (required to compile on 64bits)])],
[
	app_cv_pic="$enableval"
],[
	app_cv_pic="yes"
])

if test "$app_cv_pic" == "yes"; then
	CFLAGS="$CFLAGS -fPIC"
	CXXFLAGS="$CXXFLAGS -fPIC"
fi

dnl ---------------------------------------------------------------------------
dnl Check for headers
dnl ---------------------------------------------------------------------------

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,1,[do we have malloc.h?]))

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(Makefile)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT


