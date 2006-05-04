AC_INIT(gnome-chemistry-utils, 0.7.1)
AC_CONFIG_SRCDIR(gcu/gtkperiodic.c)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

GCU_VERSION=$VERSION

AM_CONFIG_HEADER(config.h)

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

AC_DISABLE_STATIC

AC_ISC_POSIX
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL
GNOME_DOC_INIT

GCU_VERSION_INFO=`echo $GCU_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`

AC_SUBST(GCU_VERSION_INFO)

ALL_LINGUAS="de es fr it pl ru"
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE="gchemutils-unstable"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[Package name for internationalization])

AC_ARG_ENABLE([update-databases],
	[AC_HELP_STRING(
		[--disable-update-databases],
		[do not run the update-desktop-database or update-mime-database utilities, mostly useful for package maintainers]
	)],
	[update_databases=$enableval],
	[update_databases=yes]
)
AM_CONDITIONAL(UPDATE_DATABASES,test "$update_databases" = "yes")

AC_PATH_PROG(DOXYGEN, doxygen, no)
AM_CONDITIONAL(DOXYGEN_AVAILABLE,test "$DOXYGEN" != "no")

LIBGLADE_REQUIRED_VERSION=2.4.0
GTKGLEXT_REQUIRED_VERSION=1.0.0
GNOME_PRINT_REQUIRED_VERSION=2.4.0
GNOME_VFS_REQUIRED_VERSION=2.4.0
SHARED_MIME_INFO_REQUIRED_VERSION=0.12
OPENBABEL_REQUIRED_VERSION=2.0.0
GTK_REQUIRED_VERSION=2.6.0
GNOME_OFFICE_REQUIRED_VERSION=0.3.0
CHEMICAL_MIME_DATA_REQUIRED_VERSION=0.1.0

REQUIREMENTS="libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtk+-2.0 >= $GTK_REQUIRED_VERSION \
gtkglext-1.0 >= $GTKGLEXT_REQUIRED_VERSION \
libgnomeprintui-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
gnome-vfs-2.0 >= $GNOME_VFS_REQUIRED_VERSION \
libgoffice-0.3 >= $GNOME_OFFICE_REQUIRED_VERSION \
chemical-mime-data >= $CHEMICAL_MIME_DATA_REQUIRED_VERSION \
shared-mime-info >= $SHARED_MIME_INFO_REQUIRED_VERSION \
openbabel-2.0 >= $OPENBABEL_REQUIRED_VERSION"

if test "$update_databases" = "yes"; then
	AC_CHECK_PROG(UPDATE_DESKTOP, update-desktop-database, yes)
fi
AM_CONDITIONAL(WITH_UPDATE_DESKTOP,test "$UPDATE_DESKTOP" = "yes")

AM_GCONF_SOURCE_2

PKG_CHECK_MODULES(GCU, $REQUIREMENTS)

saved_cflags=$CFLAGS
CFLAGS=$GCU_CFLAGS
AC_COMPILE_IFELSE(
	AC_LANG_PROGRAM(
		[#include <goffice/gtk/go-graph-widget.h>],
		[go_graph_widget_new(NULL)]
	),
	[new_go_graph_widget_api="yes"],
	[new_go_graph_widget_api="no"]
)
if test "$new_go_graph_widget_api" = "no"; then
	AC_DEFINE_UNQUOTED(GO_GRAPH_WIDGET_OLD_API, 1,[go_graph_widget_new has no arg])
fi
CFLAGS=$saved_cflags

GCU_CFLAGS="$GCU_CFLAGS -DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -Wall"
AC_SUBST(GCU_CFLAGS)
AC_SUBST(GCU_LIBS)

AC_LANG(C++)
AC_MSG_CHECKING([for vector::at usability])
AC_TRY_COMPILE(
	[#include <vector>],
	[std::vector<int> v; v.at(0) = 0;],
	[at_ok=yes],
	[at_ok=no]
)
AC_MSG_RESULT($at_ok)
if test "$at_ok" = "yes"; then
	AC_DEFINE_UNQUOTED(HAS_VECTOR_AT, 1, [vector::at can be used])
fi

##################################################
# Check for mozilla plugin
##################################################

MOZILLA_CFLAGS=
build_mozilla_plugin=no

test_mozilla_plugin=true
AC_ARG_ENABLE(
	[mozilla-plugin],
	[AC_HELP_STRING(
		[--enable-mozilla-plugin],
		[build a Mozilla/Firefox chemistry files viewing plugin [default=auto]]
	)],
	[test_mozilla_plugin="$enableval"]
)

if test "x$test_mozilla_plugin" != "xno"; then
	if test -z "$MOZILLA_CONFIG"; then
		AC_PATH_PROG(MOZILLA_CONFIG, mozilla-config, no)
	fi
	
	if test "x$MOZILLA_CONFIG" != "xno"; then
		_mozilla_include_dir=`mozilla-config --cflags|sed 's/-I\(.*\) .*/\1/'`
		MOZILLA_CFLAGS="-I$_mozilla_include_dir/plugin -I$_mozilla_include_dir/java -I$_mozilla_include_dir/nspr"
		mozlibdir=`mozilla-config --libs|sed 's/-L//'`
	else
		PKG_CHECK_MODULES(
			[MOZILLA],
			[mozilla-plugin],
			[build_mozilla_plugin=yes],
			[echo "no"; PKG_CHECK_MODULES(
				[MOZILLA],
				[firefox-plugin],
				[build_mozilla_plugin=yes],
				[echo "no"; AC_MSG_WARN([mozilla-config not found. Mozilla/Netscape plugin will not be built])]
			)]
		)
		mozlibdir=`pkg-config --variable=libdir mozilla-plugin`
		if test "x$mozlibdir" = "x"; then
			mozlibdir=`pkg-config --variable=libdir firefox-plugin`
		fi
	fi
	
	if test "x$MOZILLA_CFLAGS" != "x"; then
		build_mozilla_plugin=yes
	fi
fi

AC_ARG_WITH(
	[mozilla-libdir],
	[AC_HELP_STRING(
		[--with-mozilla-libdir],
		[path to mozilla/firefox libdir [default=auto]]
	)],
	[mozlibdir=$withval]
)

AM_CONDITIONAL(WITH_MOZILLA,[test "x$build_mozilla_plugin" = "xyes"])
AC_SUBST(MOZILLA_CFLAGS)
AC_SUBST(mozlibdir)

AC_LANG(C++)
AC_CHECK_HEADER(fstream,AC_DEFINE(HAVE_FSTREAM,1,[fstream present]))
AC_CHECK_HEADER(ostream,AC_DEFINE(HAVE_OSTREAM,1,[fstream present]))

if test "$prefix" = "NONE"; then
	prefix="/usr/local"
fi
eval dt=`echo $datadir`
eval sy=`echo $sysconfdir`
if test "$exec_prefix" = "NONE"; then
	lb=`echo $libdir| sed 's/exec_prefix/prefix/'`
	eval lb=`echo $lb`
	bd=`echo $bindir| sed 's/exec_prefix/prefix/'`
	eval BINDIR=`echo $bd`
	le=`echo $libexecdir| sed 's/exec_prefix/prefix/'`
	eval le=`echo $le`
else
	eval lb=`echo $libdir`
	eval BINDIR=`echo $bindir`
	eval le=`echo $libexecdir`
fi
AC_DEFINE_UNQUOTED(PREFIX, "$prefix",[Where the Gnome Chemistry Utils are to be installed])
case $srcdir in
	/*)
		AC_DEFINE_UNQUOTED(SRCDIR, "$srcdir",[Where the Gnome Chemistry Utils are compiled])
	;;
	*)
		AC_DEFINE_UNQUOTED(SRCDIR, "`pwd`/$srcdir",[Where the Gnome Chemistry Utils are compiled])
	;;
esac
AC_DEFINE_UNQUOTED(DATADIR, "$dt",[Where Gnome Chemistry Utils data are to be installed (default=${prefix}/share)])
AC_DEFINE_UNQUOTED(SYSCONFDIR, "$sy",[Where Gnome Chemistry Utils configuration files are to be installed (default=${prefix}/etc)])
AC_DEFINE_UNQUOTED(LIBDIR, "$lb",[Where Gnome Chemistry Utils libraries are to be installed (default=${prefix}/lib)])
AC_DEFINE_UNQUOTED(LIBEXECDIR, "$le",[Where Gnome Chemistry Utils program executables are to be installed (default=${exec_prefix}/libexecdir)])
AC_SUBST(BINDIR)
PIXMAPSDIR=$dt/pixmaps
AC_SUBST(PIXMAPSDIR)

AC_OUTPUT([
Makefile
gcu/Makefile
programs/Makefile
programs/3d/Makefile
programs/calc/Makefile
programs/crystal/Makefile
programs/table/Makefile
mozilla-plugin/Makefile
database/Makefile
po/Makefile.in
glade/Makefile
glade/crystal/Makefile
pixmaps/Makefile
samples/Makefile
tests/Makefile
dtds/Makefile
docs/Makefile
docs/help/Makefile
docs/help/crystal/Makefile
docs/reference/Makefile
docs/reference/Doxyfile
gcu-unstable.pc
])
