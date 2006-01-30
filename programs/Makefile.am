bin_PROGRAMS = gchemcalc gchem3d-viewer gchemtable

gchemcalc_SOURCES = gchemcalc.cc
gchemcalc_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

gchem3d_viewer_SOURCES = gchem3d-viewer.c
gchem3d_viewer_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

gchemtable_SOURCES =	\
	gchemtable-main.cc	\
	gchemtable-app.cc	\
	gchemtable-app.h	\
	gchemtable-elt.cc	\
	gchemtable-elt.h	\
	gchemtable-curve.cc	\
	gchemtable-curve.h
gchemtable_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gchem3d-viewer.desktop.in \
	gchemcalc.desktop.in	\
	gchemtable.desktop.in
desktop_DATA = \
	gchem3d-viewer.desktop \
	gchemcalc.desktop	\
	gchemtable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gchem3d-viewer.desktop \
	gchemcalc.desktop	\
	gchemtable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
