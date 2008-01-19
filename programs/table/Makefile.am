EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

LDADD = $(top_builddir)/gcu/libgcu.la

DEFS += -DDATADIR=\"$(datadir)\" \
	-DGLADEDIR=\"$(datadir)/gchemutils/glade/table\"

bin_PROGRAMS = gchemtable

gchemtable_SOURCES =	\
	gchemtable-main.cc	\
	gchemtable-app.cc	\
	gchemtable-app.h	\
	gchemtable-elt.cc	\
	gchemtable-elt.h	\
	gchemtable-curve.cc	\
	gchemtable-curve.h

desktop_in_files = gchemtable.desktop.in
desktopdir=$(datadir)/applications
desktop_DATA = gchemtable.desktop
@INTLTOOL_DESKTOP_RULE@
