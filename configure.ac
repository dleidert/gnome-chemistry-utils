AC_INIT([gnome-chemistry-utils], [0.9.1], [http://savannah.nongnu.org/bugs/?group=gchemutils])
AC_CONFIG_SRCDIR([libs/gcu/gtkperiodic.c])
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

GCU_VERSION=$VERSION

AM_CONFIG_HEADER([config.h])

AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CPPFLAGS])
AC_SUBST([LDFLAGS])

AC_DISABLE_STATIC

AC_ISC_POSIX
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_LIBTOOL
IT_PROG_INTLTOOL([0.35.0])
GNOME_DOC_INIT

GCU_VERSION_INFO=`echo $GCU_VERSION | awk -F . '{ print $1+$2":"$3":"$2 }'`

AC_SUBST([GCU_VERSION_INFO])

AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="gchemutils-unstable"
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Package name for internationalization])

AC_ARG_ENABLE([update-databases],
	[
	 AC_HELP_STRING(
		[--enable-update-databases],
		[
		 enable (or disable) the running of the update-desktop-database and update-mime-database
		 utilities (mostly useful for package maintainers) @<:@default=yes@:>@
		]
	 )
	],
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
	AC_HELP_STRING(
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
AC_SUBST(KDEMIMEDIR)

AC_PATH_PROG([DOXYGEN], [doxygen], [no])
AM_CONDITIONAL([DOXYGEN_AVAILABLE], [test "x$DOXYGEN" != "xno"])

LIBGLADE_REQUIRED_VERSION=2.4.0
GTKGLEXT_REQUIRED_VERSION=1.0.0
GNOME_PRINT_REQUIRED_VERSION=2.4.0
GNOME_VFS_REQUIRED_VERSION=2.4.0
SHARED_MIME_INFO_REQUIRED_VERSION=0.12
OPENBABEL_REQUIRED_VERSION=2.1.0
GTK_REQUIRED_VERSION=2.10.0
GNOME_OFFICE_REQUIRED_VERSION=0.4.0
CHEMICAL_MIME_DATA_REQUIRED_VERSION=0.1.94
BODR_REQUIRED_VERSION=5

REQUIREMENTS="libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtk+-2.0 >= $GTK_REQUIRED_VERSION \
gtkglext-1.0 >= $GTKGLEXT_REQUIRED_VERSION \
libgnomeprintui-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
gnome-vfs-module-2.0 >= $GNOME_VFS_REQUIRED_VERSION \
libgoffice-0.4 >= $GNOME_OFFICE_REQUIRED_VERSION \
chemical-mime-data >= $CHEMICAL_MIME_DATA_REQUIRED_VERSION \
shared-mime-info >= $SHARED_MIME_INFO_REQUIRED_VERSION \
openbabel-2.0 >= $OPENBABEL_REQUIRED_VERSION \
bodr >= $BODR_REQUIRED_VERSION"

PKG_CHECK_MODULES(GCU, $REQUIREMENTS)

PKG_CHECK_MODULES(OB22, openbabel-2.0 >= 2.2.0,
[ob22="yes"],[ob22="no"])
if test "x$ob22" = "xyes"; then
  AC_DEFINE_UNQUOTED([HAVE_OPENBABEL_2_2], 1, [Whether OpenBabel is >= 2.2.0])
fi

AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])
if test "x$GCONFTOOL" = "xno"; then
	AC_MSG_ERROR([gconftool-2 executable not found in your path - should be installed with GConf])
fi
AM_GCONF_SOURCE_2

dnl retrieve BODR data directory
bodr_pkgdatadir=`pkg-config --variable=pkgdatadir bodr`
AC_DEFINE_UNQUOTED([BODR_PKGDATADIR], ["$bodr_pkgdatadir"], [Path to Blue Obelisk Data Repository])

GCU_CFLAGS="$GCU_CFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -Wall"
AC_SUBST([GCU_CFLAGS])
AC_SUBST([GCU_LIBS])

GOFFICE_PLUGINS_DIR=`pkg-config --variable=GOffice_plugins_dir libgoffice-0.4`
AC_ARG_WITH(
	[goffice-plugins-dir],
	[AC_HELP_STRING(
		[--with-goffice-plugins-dir],
		[path to GOffice plugins directory [default=auto]]
	)],
	[GOFFICE_PLUGINS_DIR=$withval]
)

AC_SUBST(GOFFICE_PLUGINS_DIR)

##################################################
# Check for mozilla plugin
##################################################
MOZILLA_CFLAGS=
build_mozilla_plugin=no

test_mozilla_plugin=true
AC_ARG_ENABLE(
	[mozilla-plugin],
	[
	 AC_HELP_STRING(
		[--enable-mozilla-plugin],
		[build a Mozilla/Firefox chemistry files viewing plugin @<:@default=auto@:>@]
	 )
	],
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
			[mozilla-plugin],
			[build_mozilla_plugin=yes],
			[
			 PKG_CHECK_MODULES(
				[MOZILLA],
				[firefox-plugin],
				[build_mozilla_plugin=yes],
				[
				 AC_MSG_WARN([mozilla-config not found. Mozilla/Netscape plugin will not be built])
				]
			 )
			]
		)
		mozlibdir=`pkg-config --variable=libdir mozilla-plugin`
		if test -z "$mozlibdir"; then
			mozlibdir=`pkg-config --variable=libdir firefox-plugin`
		fi
	fi
	
	if test "x$MOZILLA_CFLAGS" != "x"; then
		build_mozilla_plugin=yes
	fi
fi

AC_ARG_WITH(
	[mozilla-libdir],
	[
	 AC_HELP_STRING(
		[--with-mozilla-libdir],
		[path to mozilla/firefox libdir @<:@default=auto@:>@]
	 )
	],
	[mozlibdir=$withval]
)

AM_CONDITIONAL([WITH_MOZILLA], [test "x$build_mozilla_plugin" = "xyes"])
AC_SUBST([MOZILLA_CFLAGS])
AC_SUBST([mozlibdir])

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

AC_SUBST(TESTSSRCDIR)

AC_OUTPUT([
Makefile
gcu-unstable.pc
database/Makefile
docs/Makefile
docs/help/Makefile
docs/help/3d/Makefile
docs/help/calc/Makefile
docs/help/crystal/Makefile
docs/help/paint/Makefile
docs/help/table/Makefile
docs/man/gcu_entities.dtd
docs/man/Makefile
docs/reference/Makefile
docs/reference/Doxyfile
dtds/Makefile
glade/Makefile
glade/crystal/Makefile
glade/paint/Makefile
glade/table/Makefile
libs/Makefile
libs/canvas/Makefile
libs/gcp/Makefile
libs/gcu/Makefile
plugins/Makefile
plugins/paint/Makefile
plugins/paint/arrows/Makefile
plugins/paint/atoms/Makefile
plugins/paint/bonds/Makefile
plugins/paint/cycles/Makefile
plugins/paint/residues/Makefile
plugins/paint/selection/Makefile
plugins/paint/templates/Makefile
plugins/paint/text/Makefile
plugins/paint/wikipedia/Makefile
mozilla-plugin/Makefile
goffice/Makefile
pixmaps/Makefile
po/Makefile.in
samples/Makefile
programs/Makefile
programs/3d/Makefile
programs/calc/Makefile
programs/crystal/Makefile
programs/paint/Makefile
programs/table/Makefile
templates/Makefile
templates/paint/Makefile
themes/Makefile
themes/paint/Makefile
tests/Makefile
])
