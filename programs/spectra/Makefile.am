EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade\"

bin_PROGRAMS = gspectrum-unstable

gspectrum_unstable_SOURCES = \
		application.cc \
		application.h \
		document.cc \
		document.h \
		view.cc \
		view.h \
		window.cc \
		window.h \
		gspectrum.cc

desktop_in_files = gspectrum-unstable.desktop.in

desktopdir=$(datadir)/applications
desktop_DATA = gspectrum-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
