AC_PREREQ(2.64)

AC_INIT([gnome-chemistry-utils], [0.13.98], [http://savannah.nongnu.org/bugs/?group=gchemutils],[gnome-chemistry-utils],[http://gchemutils.nongnu.org/])
AC_CONFIG_SRCDIR([libs/gcugtk/gcuperiodic.c])
AM_INIT_AUTOMAKE([1.11.1 tar-ustar no-dist-gzip dist-bzip2 dist-xz])
AM_MAINTAINER_MODE([enable])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
GCU_VERSION=$VERSION

AM_SILENT_RULES([yes])

AC_CONFIG_HEADERS([config.h])

AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([LDFLAGS])

AC_ISC_POSIX
AC_PROG_CXX
AC_HEADER_STDC

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

AC_PROG_LN_S
AM_PROG_CC_C_O
IT_PROG_INTLTOOL([0.40.6])
GNOME_DOC_INIT

GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS([yes])
GNOME_MAINTAINER_MODE_DEFINES

AC_ARG_VAR(GLIB_GENMARSHAL, [The glib-genmarshal executable.])
AC_CHECK_PROG(GLIB_GENMARSHAL, glib-genmarshal, glib-genmarshal)

GCU_VERSION_INFO=`echo $GCU_VERSION | awk -F . '{ print $1+$2":"$3":"$2 }'`
AC_SUBST([GCU_VERSION_INFO])

GCU_MAJOR_VERSION=`echo $GCU_VERSION | awk -F . '{ print $1}'`
GCU_MINOR_VERSION=`echo $GCU_VERSION | awk -F . '{ print $2}'`
GCU_MICRO_VERSION=`echo $GCU_VERSION | awk -F . '{ print $3}'`
GCU_API_MINOR_VERSION=$(((GCU_MINOR_VERSION+1)/2*2))

GCU_API_VER="$GCU_MAJOR_VERSION.$GCU_API_MINOR_VERSION"
AC_SUBST([GCU_API_VER])
AC_DEFINE_UNQUOTED([API_VERSION], ["$GCU_API_VER"], [API version])

if test "$GCU_API_MINOR_VERSION" = "$GCU_MINOR_VERSION" -o $GCU_MICRO_VERSION -ge 90; then
	gcu_is_stable="yes"
	STABILITY_POSTFIX=""
	STABILITY=""
else
	gcu_is_stable="no"
	STABILITY_POSTFIX=" ($GCU_API_VER unstable)"
	STABILITY="-unstable"
fi
AC_SUBST([STABILITY])
AC_SUBST([STABILITY_POSTFIX])
AM_CONDITIONAL([GCU_IS_STABLE], [test "x$gcu_is_stable" = "xyes"])

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="gchemutils-$GCU_API_VER"
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Package name for internationalization])

AC_ARG_ENABLE(
	[update-databases],
	AS_HELP_STRING(
		[--enable-update-databases],
		[
		 enable (or disable) the running of the update-desktop-database and update-mime-database
		 utilities (mostly useful for package maintainers) @<:@default=yes@:>@
		]
	),
	[
	 if test "x$enableval" != "xno"; then
	 	AC_PATH_PROG([UPDATE_DESKTOP_DATABASE], [update-desktop-database], [no])
		AC_PATH_PROG([UPDATE_MIME_DATABASE], [update-mime-database], [no])
	 else
	 	UPDATE_DESKTOP_DATABASE=no
		UPDATE_MIME_DATABASE=no
	 fi
	],
	[
	 AC_PATH_PROG([UPDATE_DESKTOP_DATABASE], [update-desktop-database], [no])
	 AC_PATH_PROG([UPDATE_MIME_DATABASE], [update-mime-database], [no])
	]
)

AM_CONDITIONAL([WITH_UPDATE_DESKTOP], [test "x$UPDATE_DESKTOP_DATABASE" != "xno"])
AM_CONDITIONAL([WITH_UPDATE_MIME], [test "x$UPDATE_MIME_DATABASE" != "xno"])

dnl install desktop files for kde if needed
AC_ARG_WITH(
	[kde-mime-dir],
	AS_HELP_STRING(
		[--with-kde-mime-dir],
		[
		 path to KDE's directory containing MIME .desktop files
		 (automatically detected via kde-config) @<:@default=auto or ${datadir}/mimelnk@:>@
		]
	),
	[KDEMIMEDIR=$withval],
	[
	 AC_PATH_PROG([KDECONFIG], [kde-config], [no])
	 if test "x$KDECONFIG" != "xno"; then
	 	KDEMIMEDIR="`$KDECONFIG --install mime`"
	 else
	 	KDEMIMEDIR="${datadir}/mimelnk"
	 fi	
	]
)
AC_MSG_CHECKING([for KDE MIME files (.desktop) installation location])
AC_MSG_RESULT([$KDEMIMEDIR])
AM_CONDITIONAL([WITH_KDE_MIME], [test "x$KDEMIMEDIR" != "xno"])
AC_SUBST([KDEMIMEDIR])

AC_PATH_PROG([DOXYGEN], [doxygen], [no])
AM_CONDITIONAL([DOXYGEN_AVAILABLE], [test "x$DOXYGEN" != "xno"])

libgoffice=
dnl Only 0.9.0, or later will work
for ver in 0.10 ; do
  if test "x$libgoffice" = x; then
    if pkg-config --exists libgoffice-$ver; then
      libgoffice=libgoffice-$ver
    fi
  fi
done
if test "x$libgoffice" = x; then
  # Not important.  Things will fail below.
  libgoffice=libgoffice-0.10
fi

PKG_CHECK_MODULES(shared_mime_info, [shared-mime-info >= 0.12])
PKG_CHECK_MODULES(cairo, [cairo >= 1.6.0])
PKG_CHECK_MODULES(gio, [gio-2.0 >= 2.26.0])
PKG_CHECK_MODULES(gdk_pixbuf, [gdk-pixbuf-2.0 >= 2.22.0])
PKG_CHECK_MODULES(gtk, [gtk+-3.0 >= 3.0.0])
PKG_CHECK_MODULES(xml, [libxml-2.0 >= 2.4.16])
PKG_CHECK_MODULES(gsf, [libgsf-1 >= 1.14.9])
PKG_CHECK_MODULES(goffice, [$libgoffice >= 0.9.6])
PKG_CHECK_MODULES(chemical_mime_data, [chemical-mime-data >= 0.1.94])
PKG_CHECK_MODULES(bodr, [bodr >= 5])
PKG_CHECK_MODULES(openbabel, [openbabel-2.0 >= 2.3.0])
PKG_CHECK_MODULES(x11, [x11 >= 1.0.0])

dnl Not sure we need to test for glu.h and glx.h
AC_CHECK_HEADER(GL/glu.h,,[AC_MSG_ERROR([Error, GL/glu.h not found.])])
AC_CHECK_HEADER(GL/glx.h,,[AC_MSG_ERROR([Error, GL/glx.h not found.])])

dnl GSettings related stuff
GLIB_GSETTINGS

dnl retrieve BODR data directory
bodr_pkgdatadir=`pkg-config --variable=pkgdatadir bodr`
AC_DEFINE_UNQUOTED([BODR_PKGDATADIR], ["$bodr_pkgdatadir"], [Path to Blue Obelisk Data Repository])

if test "x$gcu_is_stable" = "xyes"; then
	GCU_CFLAGS="-Wall -Wextra -Werror=format-security"
else
	GCU_CFLAGS="-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE -Wall -Wextra -Werror=format-security"
fi
AC_SUBST([GCU_CFLAGS])

dnl --without is not handled
GOFFICE_PLUGINS_DIR=`pkg-config --variable=PluginDir $libgoffice`
AC_ARG_WITH(
	[goffice-plugins-dir],
	AS_HELP_STRING(
		[--with-goffice-plugins-dir],
		[path to GOffice plugins directory [default=auto]]
	),
	[
	 if test "x$withval" != "xyes" -a "x$withval" != "xno" ; then
		GOFFICE_PLUGINS_DIR=$withval
	 fi
	]
)

dnl test some functions if needed

AC_SUBST([GOFFICE_PLUGINS_DIR])

##################################################
# Check for xulrunner plugin
##################################################
xulclient=
for pack in xulrunner seamonkey iceape firefox mozilla; do
  if test "x$xulclient" = x; then
    if pkg-config --exists $pack-plugin; then
      xulclient=$pack-plugin
    fi
  fi
done
if test "x$xulclient" = x; then
  # Not important.  Things will fail below.
  xulclient=mozilla-plugin
fi
MOZILLA_CFLAGS=
build_mozilla_plugin=no

test_mozilla_plugin=true
AC_ARG_ENABLE(
	[mozilla-plugin],
	AS_HELP_STRING(
		[--enable-mozilla-plugin],
		[build a Mozilla/Firefox chemistry files viewing plugin @<:@default=auto@:>@]
	),
	[test_mozilla_plugin="$enableval"]
)

if test "x$test_mozilla_plugin" != "xno"; then
	if test -z "$MOZILLA_CONFIG"; then
		AC_PATH_PROG([MOZILLA_CONFIG], [mozilla-config], [no])
	fi
	
	if test "x$MOZILLA_CONFIG" != "xno"; then
		MOZILLA_CFLAGS=`mozilla-config --cflags`
		mozlibdir=`mozilla-config --libs | sed 's/-L//'`
	else
		PKG_CHECK_MODULES(
			[MOZILLA],
			[$xulclient],
			[build_mozilla_plugin=yes],
			[AC_MSG_WARN([mozilla-config not found. Mozilla/Netscape plugin will not be built])]
		)
		mozlibdir=`pkg-config --variable=libdir $xulclient`
	fi
	
	if test "x$MOZILLA_CFLAGS" != "x"; then
		build_mozilla_plugin=yes

		dnl test for npfunctions.h
		saved_CPPFLAGS=$CPPFLAGS
		CPPFLAGS=$MOZILLA_CFLAGS
		AC_CHECK_HEADERS([npfunctions.h])
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([#include <npfunctions.h>], [char const *NP_GetMIMEDescription (void);])],
					[ac_moz_mime_const_char=yes], [ac_moz_mime_const_char=no])
		CPPFLAGS=$saved_CPPFLAGS
	fi
fi

AC_ARG_WITH(
	[mozilla-libdir],
	AS_HELP_STRING(
		[--with-mozilla-libdir],
		[path to mozilla/firefox/... libdir @<:@default=auto@:>@]
	),
	[mozlibdir=$withval]
)

if test -z "$mozlibdir"; then
		build_mozilla_plugin=no
		AS_WARN([Mozilla plugins directory has not been set, the plugin will not be built])
fi
AM_CONDITIONAL([WITH_MOZILLA], [test "x$build_mozilla_plugin" = "xyes"])
AC_SUBST([MOZILLA_CFLAGS])
AC_SUBST([mozlibdir])
if test "x$ac_moz_mime_const_char" = "xyes"; then
	AC_DEFINE_UNQUOTED([MOZILLA_USES_CONST_CHAR], [1], [Whether NP_GetMIMEDescription returns const char*])
fi


##################################################
# Check for gnumeric
##################################################

libspreadsheet=
for ver in 1.12 ; do
  if test "x$libspreadsheet" = x; then
    if pkg-config --exists libspreadsheet-$ver; then
      libspreadsheet=libspreadsheet-$ver
    fi
  fi
done
if test "x$libspreadsheet" = x; then
  # Not important.  Things will fail below.
  libspreadsheet=libspreadsheet-1.12
fi

PKG_CHECK_MODULES(gnumeric, [$libspreadsheet >= 1.11.6], [build_gnumeric_plugin=yes],
		[build_gnumeric_plugin=no])
dnl --without is not handled

if test "x$build_gnumeric_plugin" = "xyes"; then
GNUMERIC_PLUGINS_DIR=`pkg-config --variable=PluginDir $libspreadsheet`
gnm_version=`pkg-config --modversion $libspreadsheet`
AC_ARG_WITH(
	[gnumeric-plugins-dir],
	AS_HELP_STRING(
		[--with-gnumeric-plugins-dir],
		[path to Gnumeric plugins directory [default=auto]]
	),
	[
	 if test "x$withval" != "xyes" -a "x$withval" != "xno" ; then
		GNUMERIC_PLUGINS_DIR=$withval
	 fi
	]
)
fi
		
AM_CONDITIONAL([WITH_GNUMERIC], [test "x$build_gnumeric_plugin" = "xyes"])
AC_SUBST([GNUMERIC_PLUGINS_DIR])
AC_SUBST([gnm_version])


##################################################
# Check for various functions
##################################################
SAVE_LIBS=$LIBS
LIBS="-lm $LIBS"
AC_CHECK_FUNCS(exp10)
LIBS=$SAVE_LIBS

##################################################
# Check for manpage target applications
##################################################

GCU_PROG_XMLLINT
GCU_PROG_XSLTPROC
GCU_PROG_MAN

AC_ARG_VAR(GDK_PIXBUF_CSOURCE, [The gdk-pixbuf-csource executable.])
AC_CHECK_PROG(GDK_PIXBUF_CSOURCE, gdk-pixbuf-csource, gdk-pixbuf-csource)

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

AC_SUBST([TESTSSRCDIR])

AC_CONFIG_FILES([
Makefile
database/Makefile
docs/Makefile
docs/help/Makefile
docs/help/3d/Makefile
docs/help/calc/Makefile
docs/help/crystal/Makefile
docs/help/paint/Makefile
docs/help/spectra/Makefile
docs/help/table/Makefile
docs/man/gcu_entities.dtd
docs/man/Makefile
docs/reference/Makefile
docs/reference/Doxyfile
dtds/Makefile
libs/Makefile
libs/gccv/Makefile
libs/gcp/Makefile
libs/gcr/Makefile
libs/gcu/Makefile
libs/gcugtk/Makefile
plugins/Makefile
plugins/loaders/Makefile
plugins/loaders/cdx/Makefile
plugins/loaders/cdxml/Makefile
plugins/loaders/cif/Makefile
plugins/loaders/cml/Makefile
plugins/loaders/ctfiles/Makefile
plugins/loaders/nuts/Makefile
plugins/paint/Makefile
plugins/paint/arrows/Makefile
plugins/paint/arrows/org.gnome.gchemutils.paint.plugins.arrows.gschema.xml.in
plugins/paint/atoms/Makefile
plugins/paint/bonds/Makefile
plugins/paint/cycles/Makefile
plugins/paint/residues/Makefile
plugins/paint/selection/Makefile
plugins/paint/templates/Makefile
plugins/paint/text/Makefile
mozilla-plugin/Makefile
gnumeric/Makefile
gnumeric/plugin.xml.in
goffice/Makefile
openbabel/Makefile
pixmaps/Makefile
po/Makefile.in
samples/Makefile
schemas/Makefile
schemas/org.gnome.gchemutils.gschema.xml.in
schemas/org.gnome.gchemutils.crystal.gschema.xml.in
schemas/org.gnome.gchemutils.paint.gschema.xml.in
programs/Makefile
programs/3d/Makefile
programs/3d/gchem3d.desktop.in
programs/calc/Makefile
programs/calc/gchemcalc.desktop.in
programs/crystal/Makefile
programs/crystal/gcrystal.desktop.in
programs/paint/Makefile
programs/paint/gchempaint.desktop.in
programs/spectra/Makefile
programs/spectra/gspectrum.desktop.in
programs/table/Makefile
programs/table/gchemtable.desktop.in
templates/Makefile
templates/paint/Makefile
themes/Makefile
themes/paint/Makefile
tests/Makefile
ui/Makefile
ui/calc/Makefile
ui/crystal/Makefile
ui/libgcu/Makefile
ui/paint/Makefile
ui/table/Makefile
])
AC_OUTPUT
