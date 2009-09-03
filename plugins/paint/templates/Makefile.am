MAINTAINERCLEANFILES = Makefile.in

if GCU_NEEDS_GCONF
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(gconf_CFLAGS) \
		$(GCU_CFLAGS)
else
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)
endif

DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

templatesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
templates_LTLIBRARIES = templates.la

templates_la_LDFLAGS = -module -avoid-version
templates_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la

templates_la_SOURCES = \
	plugin.cc \
	category.cc \
	gtkcombotoolitem.c \
	templatetool.cc \
	templatetree.cc

noinst_HEADERS = \
	plugin.h \
	category.h \
	gtkcombotoolitem.h \
	templatetool.h \
	templatetree.h

gladedir = $(datadir)/gchemutils/@GCU_API_VER@/glade/paint/plugins/templates
dist_glade_DATA = \
	templates.glade \
	new-template.glade
