EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/gcu/libgcu.la
INCLUDES = -I$(top_srcdir) @GCU_CFLAGS@
LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" -DGLADEDIR=\"$(datadir)/gchemutils/glade\"

bin_PROGRAMS = gchemcalc

gchemcalc_SOURCES = gchemcalc.cc

desktopdir=$(datadir)/applications
desktop_in_files = gchemcalc.desktop.in
desktop_DATA = gchemcalc.desktop
@INTLTOOL_DESKTOP_RULE@
