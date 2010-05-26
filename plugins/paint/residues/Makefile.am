MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

residuesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
residues_LTLIBRARIES = residues.la

residues_la_LDFLAGS = -module -avoid-version
residues_la_LIBADD = \
		$(gtk_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

residues_la_SOURCES = \
	plugin.cc \
	residues-dlg.cc \
	pseudo-atom.cc

noinst_HEADERS = \
	plugin.h \
	residues-dlg.h \
	pseudo-atom.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/residues
dist_ui_DATA = residues.ui
