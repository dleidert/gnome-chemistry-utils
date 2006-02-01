bin_PROGRAMS = gchemtable-unstable

gchemtable_unstable_SOURCES =	\
	gchemtable-main.cc	\
	gchemtable-app.cc	\
	gchemtable-app.h	\
	gchemtable-elt.cc	\
	gchemtable-elt.h	\
	gchemtable-curve.cc	\
	gchemtable-curve.h
gchemtable_unstable_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu-unstable.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gchemtable-unstable.desktop.in
desktop_DATA = \
	gchemtable-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gchemtable-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
