bin_PROGRAMS = gchem3d-viewer-unstable

gchem3d_viewer_unstable_SOURCES = \
	application.cc \
	document.cc \
	main.cc	\
	view.cc	\
	window.cc

gchem3d_viewer_unstable_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@

noinst_HEADERS = \
	application.h	\
	document.h	\
	view.h	\
	window.h

DEPS = \
	$(top_builddir)/gcu/libgcu-unstable.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gchem3d-viewer-unstable.desktop.in
desktop_DATA = \
	gchem3d-viewer-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gchem3d-viewer-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
