residues_LTLIBRARIES = residues.la

residuesdir = $(libdir)/gchemutils-unstable/plugins/paint

residues_la_SOURCES = \
	plugin.cc \
	residues-dlg.cc

noinst_HEADERS = \
	plugin.h \
	residues-dlg.h

residues_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"

glade_DATA = \
	residues.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/arrows

EXTRA_DIST = \
	$(glade_DATA)

MAINTAINERCLEANFILES = Makefile.in
