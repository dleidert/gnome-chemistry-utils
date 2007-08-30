EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade\"

bin_PROGRAMS = gchemcalc-unstable

gchemcalc_unstable_SOURCES = \
	gchemcalc.cc

desktopdir=$(datadir)/applications
desktop_in_files = gchemcalc-unstable.desktop.in
desktop_DATA = gchemcalc-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
