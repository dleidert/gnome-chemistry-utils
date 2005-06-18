bin_PROGRAMS = gchemcalc-unstable gchem3d-viewer-unstable

gchemcalc_unstable_SOURCES = gchemcalc.cc
gchemcalc_unstable_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

gchem3d_viewer_unstable_SOURCES = gchem3d-viewer.c
gchem3d_viewer_unstable_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu-0.5.la

INCLUDES = @GCU_CFLAGS@
gchemcalc_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@

desktopdir=$(datadir)/applications
desktop_in_files = gchem3d-viewer-unstable.desktop.in
desktop_DATA=gchem3d-viewer-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = gchem3d-viewer-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)
