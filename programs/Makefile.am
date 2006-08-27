EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/gcu/libgcu.la
INCLUDES = @GCU_CFLAGS@
LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
DEFS +=  -DDATADIR=\"$(datadir)\" -DGLADEDIR=\"$(datadir)/gchemutils/glade\"

bin_PROGRAMS = gchemcalc gchem3d-viewer gchemtable

gchemcalc_SOURCES = gchemcalc.cc

gchem3d_viewer_SOURCES = gchem3d-viewer.c

gchemtable_SOURCES = \
	gchemtable-main.cc \
	gchemtable-app.cc \
	gchemtable-app.h \
	gchemtable-elt.cc \
	gchemtable-elt.h \
	gchemtable-curve.cc \
	gchemtable-curve.h

desktop_in_files = gchem3d-viewer.desktop.in gchemcalc.desktop.in gchemtable.desktop.in

desktopdir=$(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@
