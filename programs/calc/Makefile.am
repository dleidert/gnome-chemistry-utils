EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemcalc.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(pkgdatadir)\" \
        -DGLADEDIR=\"$(pkgdatadir)/glade\"

bin_PROGRAMS = gchemcalc

gchemcalc_SOURCES = \
		gchemcalc.cc

desktop_in_files = gchemcalc.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif
