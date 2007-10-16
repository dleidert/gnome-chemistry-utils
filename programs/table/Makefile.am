EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/table\"

bin_PROGRAMS = gchemtable-unstable

gchemtable_unstable_SOURCES = \
		gchemtable-main.cc \
		gchemtable-app.cc \
		gchemtable-app.h \
		gchemtable-elt.cc \
		gchemtable-elt.h \
		gchemtable-curve.cc \
		gchemtable-curve.h \
		gchemtable-data-allocator.cc \
		gchemtable-data-allocator.h \
		gchemtable-graph-editor.cc \
		gchemtable-graph-editor.h \
		gchemtable-data.cc \
		gchemtable-data.h

desktop_in_files = gchemtable-unstable.desktop.in

desktopdir=$(datadir)/applications
desktop_DATA = gchemtable-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
