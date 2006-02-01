bin_PROGRAMS = gchem3d-viewer-unstable

gchem3d_viewer_unstable_SOURCES = gchem3d-viewer.c
gchem3d_viewer_unstable_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@

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
