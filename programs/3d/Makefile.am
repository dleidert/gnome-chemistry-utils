EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

LDADD = $(top_builddir)/gcu/libgcu.la

DEFS += -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gchem3d-viewer

gchem3d_viewer_SOURCES = \
	application.cc \
	document.cc \
	main.cc	\
	view.cc	\
	window.cc

noinst_HEADERS = \
	application.h	\
	document.h	\
	view.h	\
	window.h

desktop_in_files = gchem3d-viewer.desktop.in
desktopdir=$(datadir)/applications
desktop_DATA = gchem3d-viewer.desktop
@INTLTOOL_DESKTOP_RULE@
