bin_PROGRAMS = gchemcalc-unstable

gchemcalc_unstable_SOURCES = gchemcalc.cc
gchemcalc_unstable_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu-unstable.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gchemcalc-unstable.desktop.in
desktop_DATA = \
	gchemcalc-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gchemcalc-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
