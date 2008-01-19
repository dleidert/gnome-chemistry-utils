EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

LDADD = $(top_builddir)/gcu/libgcu.la

DEFS += -DDATADIR=\"$(datadir)\" \
	-DGLADEDIR=\"$(datadir)/gchemutils/glade\"

bin_PROGRAMS = gchemcalc

gchemcalc_SOURCES = gchemcalc.cc

desktop_in_files = gchemcalc.desktop.in
desktopdir=$(datadir)/applications
desktop_DATA = gchemcalc.desktop
@INTLTOOL_DESKTOP_RULE@
