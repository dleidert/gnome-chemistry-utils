EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemtable.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(pkgdatadir)/glade/table\"

bin_PROGRAMS = gchemtable

gchemtable_SOURCES = \
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

desktop_in_files = gchemtable.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif
