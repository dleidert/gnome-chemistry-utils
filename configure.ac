AC_INIT(gnome-chemistry-utils, 0.2.0)
AC_CONFIG_SRCDIR(gtkperiodic/gtkperiodic.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_MAINTAINER_MODE
AC_CANONICAL_HOST

GTKP_VERSION=$VERSION
GTKCV_VERSION=$VERSION
GTKC3D_VERSION=$VERSION
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

GTKP_VERSION_INFO=`echo $GTKP_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`
GTKCV_VERSION_INFO=`echo $GTKCV_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`
GTKC3D_VERSION_INFO=`echo $GTKC3D_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`
GCU_VERSION_INFO=`echo $GCU_VERSION|awk -F . '{ print $1+$2":"$3":"$2 }'`

AC_SUBST(GTKP_VERSION_INFO)
AC_SUBST(GTKCV_VERSION_INFO)
AC_SUBST(GTKC3D_VERSION_INFO)
AC_SUBST(GCU_VERSION_INFO)

ALL_LINGUAS="fr it pl"
AM_GLIB_GNU_GETTEXT
GETTEXT_PACKAGE=AC_PACKAGE_NAME
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",[Package name for internationalization])

LIBGLADE_REQUIRED_VERSION=1.99.5
GTKGL_REQUIRED_VERSION=1.99.0
GTKGLEXT_REQUIRED_VERSION=0.6.0
GNOME_PRINT_REQUIRED_VERSION=2.1.3
GNOME_VFS_REQUIRED_VERSION=2.0.0
BONOBOUI_REQUIRED_VERSION=2.2.0
LIBGNOMEUI_REQUIRED_VERSION=2.0.0

gl_flags=
gtkcrystalviewer="-lgtkcrystalviewer"

PKG_CHECK_MODULES(GCU, \
libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtkglext-1.0 >= $GTKGLEXT_REQUIRED_VERSION \
libgnomeprint-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
libbonoboui-2.0 >= $BONOBOUI_REQUIRED_VERSION \
, have_gtkgl="yes", have_gtk_gl="no")
if test "$have_gtkgl" = "yes"; then
AC_DEFINE_UNQUOTED(HAVE_GTKGLEXT, 1,[Compile with GtlGLExt])
gl_flags="-DUSE_GTKGLEXT"
else
PKG_CHECK_MODULES(GCU, \
libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
gtkgl-2.0 >= $GTKGL_REQUIRED_VERSION \
libgnomeprint-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
libbonoboui-2.0 >= $BONOBOUI_REQUIRED_VERSION \
,have_gtkgl="yes", have_gtk_gl="no")
if test "$have_gtkgl" = "yes"; then
AC_DEFINE_UNQUOTED(HAVE_GTKGLAREA,1,[Compile with GtlGLArea])
gl_flags="-DUSE_GTKGLAREA"
else
PKG_CHECK_MODULES(GCU, \
libglade-2.0 >= $LIBGLADE_REQUIRED_VERSION \
libgnomeprint-2.2 >= $GNOME_PRINT_REQUIRED_VERSION \
libbonoboui-2.0 >= $BONOBOUI_REQUIRED_VERSION \
)
gtkcrystalviewer=
fi
fi

AC_SUBST(gl_flags)
AC_SUBST(gtkcrystalviewer)
AM_CONDITIONAL(WITH_GTKGL,test "$have_gtkgl" = "yes")

dnl test if openbabel is present and chem3d can be built
if test "$have_gtkgl" = "yes"; then
	
	OPENBABEL_REQUIRED_VERSION=1.100.1
	PKG_CHECK_MODULES(GC3D, \
		gnome-vfs-2.0 >= $GNOME_VFS_REQUIRED_VERSION \
		libgnomeui-2.0 >= $LIBGNOMEUI_REQUIRED_VERSION \
		openbabel >= $OPENBABEL_REQUIRED_VERSION, \
		have_openbabel="yes", \
		have_openbabel="no" && \
		AC_MSG_WARN(*** GtkChem3DViewer widget needs gnome-vfs-2.0 and OpenBabel will not be built ***))
	AC_SUBST(GC3D_CFLAGS)
	AC_SUBST(GC3D_LIBS)

fi

AM_CONDITIONAL(WITH_OPENBABEL,test "$have_openbabel" = "yes")

GCU_CFLAGS="$GCU_CFLAGS -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED \
-DLIBGLADE_DISABLE_DEPRECATED"
AC_SUBST(GCU_CFLAGS)
AC_SUBST(GCU_LIBS)

# This is a check for gtk-doc which you can insert into your configure.in.
# You shouldn't need to change it at all.


##################################################
# Check for gtk-doc.
##################################################

AC_ARG_WITH(html-dir, [  --with-html-dir=PATH path to installed docs ])

if test "x$with_html_dir" = "x" ; then
  HTML_DIR='${datadir}/gtk-doc/html'
else
  HTML_DIR=$with_html_dir
fi

AC_SUBST(HTML_DIR)

gtk_doc_min_version=1.0
AC_MSG_CHECKING([gtk-doc version >= $gtk_doc_min_version])
if pkg-config --atleast-version=$gtk_doc_min_version gtk-doc; then
  AC_MSG_RESULT(yes)
  GTKDOC=true
else
  AC_MSG_RESULT(no)
  GTKDOC=false
fi

dnl Let people disable the gtk-doc stuff.
AC_ARG_ENABLE(gtk-doc, [  --enable-gtk-doc  Use gtk-doc to build documentation [default=auto]], enable_gtk_doc="$enableval", enable_gtk_doc=auto)

if test x$enable_gtk_doc = xauto ; then
  if test x$GTKDOC = xtrue ; then
    enable_gtk_doc=yes
  else
    enable_gtk_doc=no 
  fi
fi

AM_CONDITIONAL(ENABLE_GTK_DOC, test x$enable_gtk_doc = xyes)

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
AC_DEFINE_UNQUOTED(SRCDIR, "`pwd`",[Where the Gnome Chemistry Utils are compiled])
AC_DEFINE_UNQUOTED(DATADIR, "$dt",[Where Gnome Chemistry Utils data are to be installed (default=${prefix}/share)])
AC_DEFINE_UNQUOTED(SYSCONFDIR, "$sy",[Where Gnome Chemistry Utils configuration files are to be installed (default=${prefix}/etc)])
AC_DEFINE_UNQUOTED(LIBDIR, "$lb",[Where Gnome Chemistry Utils libraries are to be installed (default=${prefix}/lib)])
AC_SUBST(BINDIR)
PIXMAPSDIR=$dt/pixmaps
AC_SUBST(PIXMAPSDIR)

AC_OUTPUT([
Makefile
chemistry/Makefile
crystalviewer/Makefile
gtkperiodic/Makefile
chem3d/Makefile
po/Makefile.in
glade/Makefile
tests/Makefile
dtds/Makefile
docs/Makefile
docs/reference/Makefile
docs/reference/html/Makefile
gcu.pc
chem3d/gchem3d.server.in
])
