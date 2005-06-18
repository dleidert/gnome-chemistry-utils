AC_INIT(gnome-chemistry-utils, 0.5.0)
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

GCU_VERSION_INFO=`echo $GCU_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`

AC_SUBST(GCU_VERSION_INFO)

ALL_LINGUAS="de fr it pl"
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[Package name for internationalization])

AC_ARG_ENABLE([update-databases],
	AC_HELP_STRING([--disable-update-databases],[do not run the update-desktop-database or update-mime-database utilities, mostly useful for package maintainers]),[
	update_databases=$enableval
	],[
	update_databases=yes]
)
AM_CONDITIONAL(UPDATE_DATABASES,test "$update_databases" = "yes")

AC_PATH_PROG(DOXYGEN, doxygen, no)
AM_CONDITIONAL(DOXYGEN_AVAILABLE,test "$DOXYGEN" != "no")

LIBGLADE_REQUIRED_VERSION=2.4.0
GTKGLEXT_REQUIRED_VERSION=1.0.0
GNOME_PRINT_REQUIRED_VERSION=2.4.0
GNOME_VFS_REQUIRED_VERSION=2.4.0
LIBGNOMEUI_REQUIRED_VERSION=2.4.0
SHARED_MIME_INFO_REQUIRED_VERSION=0.12
OPENBABEL_REQUIRED_VERSION=1.100.2

REQUIREMENTS="libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtkglext-1.0 >= $GTKGLEXT_REQUIRED_VERSION \
libgnomeprint-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
gnome-vfs-2.0 >= $GNOME_VFS_REQUIRED_VERSION \
libgnomeui-2.0 >= $LIBGNOMEUI_REQUIRED_VERSION \
openbabel >= $OPENBABEL_REQUIRED_VERSION"

if test "$update_databases" = "yes"; then
REQUIREMENTS="$REQUIREMENTS shared-mime-info >= $SHARED_MIME_INFO_REQUIRED_VERSION";
AC_CHECK_PROG(UPDATE_DESKTOP, update-desktop-database, yes)
fi
AM_CONDITIONAL(WITH_UPDATE_DESKTOP,test "$UPDATE_DESKTOP" = "yes")

PKG_CHECK_MODULES(GCU, $REQUIREMENTS)

AC_SUBST(GCU_CFLAGS)
AC_SUBST(GCU_LIBS)

AC_LANG(C++)
AC_MSG_CHECKING([for vector::at usability])
AC_TRY_COMPILE([#include <vector>],
      [std::vector<int> v; v.at(0) = 0;],
      at_ok=yes,
      at_ok=no)
AC_MSG_RESULT($at_ok)
if test "$at_ok" = "yes"; then
	AC_DEFINE_UNQUOTED(HAS_VECTOR_AT, 1, [vector::at can be used])
fi

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
else
	eval lb=`echo $libdir`
	eval BINDIR=`echo $bindir`
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
AC_SUBST(BINDIR)
PIXMAPSDIR=$dt/pixmaps
AC_SUBST(PIXMAPSDIR)

AC_OUTPUT([
Makefile
gcu/Makefile
programs/Makefile
database/Makefile
po/Makefile.in
glade/Makefile
tests/Makefile
dtds/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/Doxyfile
gcu-0.5.pc
])
