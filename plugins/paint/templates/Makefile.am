templates_LTLIBRARIES = templates.la

templatesdir = $(libdir)/gchemutils-unstable/plugins/paint

templates_la_SOURCES = \
	plugin.cc \
	gtkcombotoolitem.c \
	templatetool.cc \
	templatetree.cc \
	category.cc

noinst_HEADERS = \
	plugin.h \
	gtkcombotoolitem.h \
	templatetool.h \
	templatetree.h \
	category.h

glade_DATA = \
	templates.glade \
	new-template.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/templates

EXTRA_DIST=$(glade_DATA)

templates_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS +=  -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"
