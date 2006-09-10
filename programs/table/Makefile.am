EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gchemtable-unstable

DEPS = $(top_builddir)/gcu/libgcu-unstable.la
INCLUDES = -I$(top_srcdir) @GCU_CFLAGS@
LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/table\"

gchemtable_unstable_SOURCES =	\
	gchemtable-main.cc	\
	gchemtable-app.cc	\
	gchemtable-app.h	\
	gchemtable-elt.cc	\
	gchemtable-elt.h	\
	gchemtable-curve.cc	\
	gchemtable-curve.h

desktopdir=$(datadir)/applications
desktop_in_files = gchemtable-unstable.desktop.in
desktop_DATA = gchemtable-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
