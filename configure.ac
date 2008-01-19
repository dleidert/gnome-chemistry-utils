AC_INIT([gnome-chemistry-utils], [0.8.6], [http://savannah.nongnu.org/bugs/?group=gchemutils])
AC_CONFIG_SRCDIR([gcu/gtkperiodic.c])
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

AM_CONFIG_HEADER([config.h])

GCU_VERSION=$VERSION
GCU_VERSION_INFO=`echo $GCU_VERSION | awk -F . '{ print $1+$2":"$3":"$2 }'`
AC_SUBST([GCU_VERSION_INFO])

##################################################
# init
##################################################

AC_DISABLE_STATIC

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_HEADER_STDC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
GNOME_DOC_INIT

AC_LANG(C++)
AC_CHECK_HEADERS([fstream ostream sstream])

##################################################
# i18n
##################################################

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="gchemutils"
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Package name for internationalization])
IT_PROG_INTLTOOL([0.35.0])

##################################################
# windows systems
##################################################

case "$host" in
	*-*-mingw* | *cygwin* | *windows*)
		native_win32=yes
	;;		
esac

case "$host_os" in
	windows* | cygwin* | mingw*)
		AC_DEFINE([OS_WIN32], [1], [Define to 1 if you have a Cygwin/Windows OS.])
		AC_MSG_NOTICE([platform is... Windows/Cygwin/Mingw])
	;;
esac

AM_CONDITIONAL([OS_WIN32], [test "x$native_win32" = "xyes"])

##################################################
# configure options
##################################################

AC_ARG_ENABLE(
	[update-databases],
	AC_HELP_STRING(
		[--enable-update-databases],
		[
		 enable (or disable) the running of the update-desktop-database and update-mime-database
		 utilities (mostly useful for package maintainers) @<:@default=yes@:>@
		]
	)
)
if test "x$enable_update_databases" != "xno"; then
	AC_PATH_PROG([UPDATE_DESKTOP_DATABASE], [update-desktop-database], [no])
	AC_PATH_PROG([UPDATE_MIME_DATABASE], [update-mime-database], [no])
else
	UPDATE_DESKTOP_DATABASE=no
	UPDATE_MIME_DATABASE=no
fi
AM_CONDITIONAL([WITH_UPDATE_MIME], [test "x$UPDATE_MIME_DATABASE" != "xno"])
AM_CONDITIONAL([WITH_UPDATE_DESKTOP], [test "x$UPDATE_DESKTOP_DATABASE" != "xno"])

dnl install desktop files for kde if needed
AC_ARG_WITH(
	[kde-mime-dir],
	AC_HELP_STRING(
		[--with-kde-mime-dir],
		[
		 path to KDE's directory containing MIME .desktop files
		 (automatically detected via kde-config) @<:@default=auto or ${datadir}/mimelnk@:>@
		]
	),,
	[
	 AC_PATH_PROG([KDECONFIG], [kde-config], [no])
	 if test "x$KDECONFIG" != "xno"; then
	 	with_kde_mime_dir="`$KDECONFIG --install mime`"
	 else
	 	with_kde_mime_dir="${datadir}/mimelnk"
	 fi	
	]
)
AC_MSG_CHECKING([for KDE MIME files (.desktop) installation location])
AC_MSG_RESULT([$with_kde_mime_dir])
AM_CONDITIONAL([WITH_KDE_MIME], [test "x$with_kde_mime_dir" != "xno"])
AC_SUBST([with_kde_mime_dir])

##################################################
# pkgconfig
##################################################

PKG_PROG_PKG_CONFIG

LIBGLADE_REQUIRED_VERSION=2.4.0
GTKGLEXT_REQUIRED_VERSION=1.0.0
GNOME_PRINT_REQUIRED_VERSION=2.4.0
GNOME_VFS_REQUIRED_VERSION=2.4.0
SHARED_MIME_INFO_REQUIRED_VERSION=0.12
OPENBABEL_REQUIRED_VERSION=2.1.0
GTK_REQUIRED_VERSION=2.10.0
GNOME_OFFICE_REQUIRED_VERSION=0.4.0
GNOME_OFFICE_MAX_VERSION=0.5.0
CHEMICAL_MIME_DATA_REQUIRED_VERSION=0.1.94
BODR_REQUIRED_VERSION=5

REQUIREMENTS="libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtk+-2.0 >= $GTK_REQUIRED_VERSION \
gtkglext-1.0 >= $GTKGLEXT_REQUIRED_VERSION \
libgnomeprintui-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
gnome-vfs-2.0 >= $GNOME_VFS_REQUIRED_VERSION \
libgoffice-0.4 >= $GNOME_OFFICE_REQUIRED_VERSION \
libgoffice-0.4 < $GNOME_OFFICE_MAX_VERSION \
chemical-mime-data >= $CHEMICAL_MIME_DATA_REQUIRED_VERSION \
shared-mime-info >= $SHARED_MIME_INFO_REQUIRED_VERSION \
openbabel-2.0 >= $OPENBABEL_REQUIRED_VERSION \
bodr >= $BODR_REQUIRED_VERSION"

PKG_CHECK_MODULES([GCU], [$REQUIREMENTS])

AC_SUBST([GCU_CFLAGS])
AC_SUBST([GCU_LIBS])

##################################################
# FLAGS/LIBS
##################################################

AC_SUBST([CFLAGS])
CFLAGS="$CFLAGS -Wall $GCU_CFLAGS"
AC_SUBST([CXXFLAGS])
CXXFLAGS="$CXXFLAGS -Wall $GCU_CFLAGS"
AC_SUBST([CPPFLAGS])
AC_SUBST([LDFLAGS])
if test "x$native_win32" = "xyes"; then
	LDFLAGS="$LDFLAGS -no-undefined"
fi
AC_SUBST([LIBS])
LIBS="$LIBS $GCU_LIBS"

##################################################
# xulrunner
##################################################

AC_ARG_VAR(
        [MOZILLA_CONFIG],
        [
         The 'mozilla-config' binary with path.
         Use it to define or override the location of 'mozilla-config'.
        ]
)
AC_ARG_VAR(
        [XULRUNNER_CLIENT],
        [
         Set to xulrunner or a compliant browser. The script will automatically
         test for xulrunner seamonkey firefox and mozilla. You can also use this
         veriable to override the detection.
        ]
)

for pack in xulrunner seamonkey firefox mozilla; do
	if test "x$XULRUNNER_CLIENT" = "x"; then
		if pkg-config --exists $pack-plugin; then
			XULRUNNER_CLIENT=$pack-plugin
		fi
	fi
done
if test "x$XULRUNNER_CLIENT" = x; then
  # Not important.  Things will fail below.
  libgoffice=mozilla-plugin
fi
MOZILLA_CFLAGS=
build_mozilla_plugin=no

enable_mozilla_plugin=yes
AC_ARG_ENABLE(
	[mozilla-plugin],
	AC_HELP_STRING(
		[--enable-mozilla-plugin],
		[build a Mozilla/Firefox chemistry files viewing plugin @<:@default=auto@:>@]
	)
)

if test "x$enable_mozilla_plugin" != "xno"; then
	if test -z "$MOZILLA_CONFIG"; then
		AC_PATH_PROG([MOZILLA_CONFIG], [mozilla-config], [no])
	fi
	
	if test "x$MOZILLA_CONFIG" != "xno"; then
		MOZILLA_CFLAGS=`mozilla-config --cflags`
		default_mozilla_libdir=`mozilla-config --libs | sed 's/-L//'`
	elif test -n "$XULRUNNER_CLIENT"; then
		PKG_CHECK_MODULES(
			[MOZILLA],
			[$XULRUNNER_CLIENT],
			[build_mozilla_plugin=yes],
			[build_mozilla_plugin=no]
		)
		default_mozilla_libdir=`pkg-config --variable=libdir $XULRUNNER_CLIENT`
	fi
	
	if test "x$MOZILLA_CFLAGS" != "x"; then
		build_mozilla_plugin=yes
	else
		AC_MSG_WARN([mozilla-config or $XULRUNNER_CLIENT.pc not found.])
		AC_MSG_WARN([Mozilla/Netscape plugin will not be built])
	fi
fi

AC_ARG_WITH(
	[mozilla-libdir],
	AC_HELP_STRING(
		[--with-mozilla-libdir],
		[path to mozilla/firefox/... libdir @<:@default=auto@:>@]
	),
	[
	 if test "x$withval" = "xno"; then
	 	AC_MSG_WARN([Use --disable-mozilla-plugin to not build the package.])
	 	with_mozilla_libdir=$default_mozilla_libdir
	 fi
	],
	[with_mozilla_libdir=$default_mozilla_libdir]
)

AM_CONDITIONAL([WITH_MOZILLA], [test "x$build_mozilla_plugin" = "xyes"])
AC_SUBST([MOZILLA_CFLAGS])
AC_SUBST([with_mozilla_libdir])

##################################################
# misc
##################################################

dnl check if goffice has gnome support
AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[
#include <goffice/goffice-features.h>
#ifndef GOFFICE_WITH_GNOME
#error no gnome support present
#endif
]])],
	[],
	[AC_MSG_ERROR([you need a libgoffice version with gnome support])]
)

AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])
if test "x$GCONFTOOL" = "xno"; then
	AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi
AM_GCONF_SOURCE_2

dnl retrieve BODR data directory
bodr_pkgdatadir=`pkg-config --variable=pkgdatadir bodr`
AC_DEFINE_UNQUOTED([BODR_PKGDATADIR], ["$bodr_pkgdatadir"], [Path to Blue Obelisk Data Repository])

AC_PATH_PROG([DOXYGEN], [doxygen], [no])
AM_CONDITIONAL([DOXYGEN_AVAILABLE], [test "x$DOXYGEN" != "xno"])

AC_LANG(C++)
AC_MSG_CHECKING([for vector::at usability])
AC_TRY_COMPILE(
	[#include <vector>],
	[std::vector<int> v; v.at(0) = 0;],
	[
	 AC_DEFINE([HAS_VECTOR_AT], [1], [Define to 1 if vector::at can be used.])
	 AC_MSG_RESULT([yes])
	],
	[AC_MSG_RESULT([no])]
)

##################################################
# manpage target
##################################################

GCU_PROG_XMLLINT
GCU_PROG_XSLTPROC
GCU_PROG_MAN

##################################################
# Path checks
##################################################

case $srcdir in
	/*)
		TESTSSRCDIR="$srcdir/tests"
	;;
	*)
		TESTSSRCDIR=`pwd`"/$srcdir/tests"
	;;
esac

AC_SUBST(TESTSSRCDIR)

##################################################
# output
##################################################

AC_CONFIG_FILES([
Makefile
gcu.pc
database/Makefile
docs/Makefile
docs/help/Makefile
docs/help/3d/Makefile
docs/help/calc/Makefile
docs/help/crystal/Makefile
docs/help/table/Makefile
docs/man/gcu_entities.dtd
docs/man/Makefile
docs/reference/Makefile
docs/reference/Doxyfile
dtds/Makefile
gcu/Makefile
glade/Makefile
glade/crystal/Makefile
glade/table/Makefile
mozilla-plugin/Makefile
pixmaps/Makefile
po/Makefile.in
samples/Makefile
programs/Makefile
programs/3d/Makefile
programs/calc/Makefile
programs/crystal/Makefile
programs/table/Makefile
tests/Makefile
])
AC_OUTPUT
