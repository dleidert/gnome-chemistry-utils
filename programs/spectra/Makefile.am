EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gspectrum.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DGLADEDIR=\"$(pkgdatadir)/glade\"

bin_PROGRAMS = gspectrum

gspectrum_SOURCES = \
		application.cc \
		application.h \
		document.cc \
		document.h \
		view.cc \
		view.h \
		window.cc \
		window.h \
		gspectrum.cc

desktop_in_files = gspectrum.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif
