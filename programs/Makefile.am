bin_PROGRAMS = gchemcalc-unstable gchem3d-viewer-unstable gchemtable-unstable

gchemcalc_unstable_SOURCES = gchemcalc.cc
gchemcalc_unstable_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

gchem3d_viewer_unstable_SOURCES = gchem3d-viewer.c
gchem3d_viewer_unstable_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

gchemtable_unstable_SOURCES =	\
	gchemtable-main.cc	\
	gchemtable-app.cc	\
	gchemtable-app.h	\
	gchemtable-elt.cc	\
	gchemtable-elt.h
gchemtable_unstable_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu-0.5.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gchem3d-viewer-unstable.desktop.in \
	gchemcalc-unstable.desktop.in	\
	gchemtable-unstable.desktop.in
desktop_DATA = \
	gchem3d-viewer-unstable.desktop \
	gchemcalc-unstable.desktop	\
	gchemtable-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gchem3d-viewer-unstable.desktop \
	gchemcalc-unstable.desktop	\
	gchemtable-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
