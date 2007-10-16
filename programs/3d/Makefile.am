EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gchem3d-viewer-unstable

gchem3d_viewer_unstable_SOURCES = \
		application.cc \
		application.h \
		document.cc \
		document.h \
		main.cc	\
		view.cc	\
		view.h \
		window.cc \
		window.h

desktop_in_files = gchem3d-viewer-unstable.desktop.in

desktopdir=$(datadir)/applications
desktop_DATA = gchem3d-viewer-unstable.desktop
@INTLTOOL_DESKTOP_RULE@
