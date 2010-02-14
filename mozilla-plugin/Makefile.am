MAINTAINERCLEANFILES = Makefile.in

if WITH_MOZILLA
plugin_LTLIBRARIES = libmozgcu.la
libexec_PROGRAMS = chem-viewer
plugindir = $(mozlibdir)/plugins/
else
plugin_LTLIBRARIES =
libexec_PROGRAMS =
plugindir =
endif

DEFS += -DLIBEXECDIR=\"$(libexecdir)\" -DDATADIR=\"$(datadir)\"

libmozgcu_la_SOURCES = moz-plugin.c
libmozgcu_la_CPPFLAGS = $(MOZILLA_CFLAGS)
libmozgcu_la_LDFLAGS = -module -avoid-version 
libmozgcu_la_LIBADD = $(MOZILLA_LIBS)

chem_viewer_SOURCES = chem-viewer.cc
chem_viewer_LDADD = $(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(goffice_LIBS) \
	$(gtk_LIBS) \
	$(x11_LIBS) \
	$(xml_LIBS)

if GCU_NEEDS_GCONF
chem_viewer_LDADD += \
		$(gnome_vfs_CFLAGS)
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(glade_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(gnome_vfs_CFLAGS) \
		$(GCU_CFLAGS)
else
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(glade_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)
endif
