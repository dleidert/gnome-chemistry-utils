EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gchem3d-viewer-unstable

gchem3d_viewer_unstable_SOURCES = \
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

desktopdir=$(datadir)/applications
desktop_in_files = gchem3d-viewer-unstable.desktop.in
desktop_DATA = gchem3d-viewer-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
