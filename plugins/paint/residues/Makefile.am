MAINTAINERCLEANFILES = Makefile.in

if GCU_NEEDS_GCONF
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(glade_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(gconf_CFLAGS) \
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

DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

residuesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
residues_LTLIBRARIES = residues.la

residues_la_LDFLAGS = -module -avoid-version
residues_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la

residues_la_SOURCES = \
	plugin.cc \
	residues-dlg.cc \
	pseudo-atom.cc

noinst_HEADERS = \
	plugin.h \
	residues-dlg.h \
	pseudo-atom.h

gladedir = $(datadir)/gchemutils/@GCU_API_VER@/glade/paint/plugins/residues
dist_glade_DATA = residues.glade
