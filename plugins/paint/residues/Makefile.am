MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

residuesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
residues_LTLIBRARIES = residues.la

residues_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

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
