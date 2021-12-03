dnl
dnl Software pw3270, desenvolvido com base nos códigos fontes do  C3270  e  X3270
dnl (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
dnl aplicativos mainframe. Registro no INPI sob o nome G3270.
dnl
dnl Copyright (C) <2008> <Banco do Brasil S.A.>
dnl
dnl Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
dnl os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
dnl Free Software Foundation.
dnl
dnl Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
dnl GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
dnl A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
dnl obter mais detalhes.
dnl
dnl Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
dnl programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple

dnl Place, Suite 330, Boston, MA, 02111-1307, USA
dnl
dnl Contatos:
dnl
dnl perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
dnl erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
dnl

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.61)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([libv3270], [5.4], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Recommended for gtk-doc (https://developer.gnome.org/gtk-doc-manual/stable/settingup_autoconf.html.en)
AC_CONFIG_MACRO_DIR(m4)

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

		INSTALL_PACKAGES="linux-lib ${INSTALL_PACKAGES}"

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

AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_VERSION, $app_vrs_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_VERSION, $app_vrs_minor)

AC_SUBST(PACKAGE_MAJOR_VERSION,$app_vrs_major)
AC_SUBST(PACKAGE_MINOR_VERSION,$app_vrs_minor)

AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"libv3270-$app_vrs_major.$app_vrs_minor",[The gettext package name.])
AC_SUBST(GETTEXT_PACKAGE,"libv3270-$app_vrs_major.$app_vrs_minor")

AC_ARG_WITH([release], [AS_HELP_STRING([--with-release], [Set release])], [ app_cv_release="$withval" ],[ app_cv_release="`date +%-y`.`date +%-m`.`date +%-d`" ])

app_rls_major=$(echo $app_cv_release.0.0 | cut -d. -f1)
app_rls_minor=$(echo $app_cv_release.0.0 | cut -d. -f2)

AC_DEFINE_UNQUOTED(PACKAGE_RELEASE, "$app_cv_release")
AC_DEFINE_UNQUOTED(PACKAGE_MAJOR_RELEASE, $app_rls_major)
AC_DEFINE_UNQUOTED(PACKAGE_MINOR_RELEASE, $app_rls_minor)

AC_SUBST(PACKAGE_RELEASE,$app_cv_release)
AC_SUBST(PACKAGE_MAJOR_RELEASE,$app_rls_major)
AC_SUBST(PACKAGE_MINOR_RELEASE,$app_rls_minor)

date="date"
test -z "$SOURCE_DATE_EPOCH" || date="$date -u -d@$SOURCE_DATE_EPOCH"
rpq_build_date=`LANG=C $date`
rpq_timestamp=`$date +%Y%m%d%H%M%S`
rpq_revision=`$date +"%Y%m%d"`
sccs_date=`$date +%Y/%m/%d`
sccs_user=$USER

AC_DEFINE_UNQUOTED(RPQ_BUILD_DATE,$rpq_build_date)
AC_DEFINE_UNQUOTED(RPQ_TIMESTAMP,$rpq_timestamp)
AC_DEFINE_UNQUOTED(RPQ_REVISION,$rpq_revision)

AC_DEFINE_UNQUOTED(SCCS_USER,$USER)
AC_DEFINE_UNQUOTED(SCCS_DATE,$sccs_date)

case "$host" in
	*-mingw32|*-pc-msys)
		AC_SUBST(SONAME,libv3270.dll)
		;;

 	s390x-*)
		AC_SUBST(SONAME,libv3270.so)
		;;

 	*)
		AC_SUBST(SONAME,libv3270.dylib.$app_vrs_major.$app_vrs_minor)

esac

dnl ---------------------------------------------------------------------------
dnl Check for static library options
dnl ---------------------------------------------------------------------------

AC_ARG_ENABLE([static],
 	[AS_HELP_STRING([--enable-static], [Enable build and install the static library])],
[
 	app_cv_static="$enableval"
],[
	AC_MSG_NOTICE([Keep default selection for static library.])
])

if test "$app_cv_static" == "yes"; then
	INSTALL_PACKAGES="$INSTALL_PACKAGES static"
fi

dnl ---------------------------------------------------------------------------
dnl Check for libm (Required for spinner)
dnl ---------------------------------------------------------------------------

AC_SEARCH_LIBS( [sin], [m], AC_DEFINE(HAVE_LIBM), AC_MSG_ERROR([libm not present.]))

dnl ---------------------------------------------------------------------------
dnl Check for libintl
dnl ---------------------------------------------------------------------------

INTL_LIBS=""

AC_PATH_TOOL([XGETTEXT], [xgettext], [no])
AC_PATH_TOOL([MSGCAT], [msgcat], [no])
AC_PATH_TOOL([MSGINIT], [msginit], [no])
AC_PATH_TOOL([MSGMERGE], [msgmerge], [no])
AC_PATH_TOOL([MSGFMT], [msgfmt], [no])
AC_PATH_TOOL([VALGRIND], [valgrind], [no])
AC_PATH_TOOL([GENMARSHAL], [glib-genmarshal], [glib-genmarshal])
AC_PATH_TOOL([STRIP], [strip], [true])

AC_CHECK_HEADER(libintl.h, [
	AC_DEFINE(HAVE_LIBINTL, 1)

	case "$host" in
	*-mingw32)
		INTL_LIBS="-lintl"
		;;

	*-apple-darwin*)
		INTL_LIBS="-lintl"
		;;

	*)
		INTL_LIBS=""

	esac

])

AC_SUBST(INTL_LIBS)

dnl ---------------------------------------------------------------------------
dnl Check for LIB3270
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [LIB3270], [lib3270], AC_DEFINE(HAVE_LIB3270), AC_MSG_ERROR([LIB3270 not present.]))

AC_SUBST(LIB3270_LIBS)
AC_SUBST(LIB3270_CFLAGS)

AC_ARG_WITH([product-name], [AS_HELP_STRING([--with-product-name], [Set product name])], [ app_cv_product="$withval" ],[ app_cv_product=`pkg-config --variable=product_name lib3270` ])
AC_DEFINE_UNQUOTED(PRODUCT_NAME, $app_cv_product)
AC_SUBST(PRODUCT_NAME,$app_cv_product)

dnl ---------------------------------------------------------------------------
dnl Check for GTK
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GTK], [gtk+-3.0 gmodule-2.0], AC_DEFINE(HAVE_GTK), AC_MSG_ERROR([GTK not present.]))

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_MODVERSION,`$PKG_CONFIG --modversion gtk+-3.0`)

dnl ---------------------------------------------------------------------------
dnl Check for GLIB
dnl ---------------------------------------------------------------------------

PKG_CHECK_MODULES( [GLIB], [glib-2.0], AC_DEFINE(HAVE_GLIB), AC_MSG_ERROR([GLIB not present.] ))
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

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

AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

AC_CHECK_FUNCS(getaddrinfo, AC_DEFINE(HAVE_GETADDRINFO) )
AC_CHECK_FUNC(vasprintf, AC_DEFINE(HAVE_VASPRINTF) )
AC_CHECK_FUNC(strtok_r, AC_DEFINE(HAVE_STRTOK_R) )
AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R) )

AC_ARG_WITH([inet-ntop], [AS_HELP_STRING([--with-inet-ntop], [Assume that inet_nto() is available])], [ app_cv_inet_ntop="$withval" ],[ app_cv_inet_ntop="auto" ])

if test "$app_cv_inet_ntop" == "auto"; then

	case "$host" in
	*-mingw32|*-pc-msys)
		app_cv_inet_ntop="yes"
		;;

	*)
		AC_CHECK_FUNC(inet_ntop, app_cv_inet_ntop="yes" )

	esac

fi

if test "$app_cv_inet_ntop" == "yes"; then
	AC_DEFINE(HAVE_INET_NTOP)
fi

dnl ---------------------------------------------------------------------------
dnl Directory config
dnl ---------------------------------------------------------------------------

AC_ARG_WITH([build-dir], [AS_HELP_STRING([--with-build-dir], [Setup build path])], [ app_cv_buildir="$withval" ],[ app_cv_buildir="$ac_pwd" ])
AC_SUBST(BASEDIR,$app_cv_buildir)

dnl ---------------------------------------------------------------------------
dnl Configure which files to generate.
dnl ---------------------------------------------------------------------------

AC_SUBST(INSTALL_PACKAGES)

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(locale/Makefile)
AC_CONFIG_FILES(sdk/libv3270.pc)
AC_CONFIG_FILES(glade/v3270.xml)

dnl ---------------------------------------------------------------------------
dnl Output the generated config.status script.
dnl ---------------------------------------------------------------------------
AC_OUTPUT


