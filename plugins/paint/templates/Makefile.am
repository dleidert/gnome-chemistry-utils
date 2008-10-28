MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(pkgdatadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

templatesdir = $(pkglibdir)/plugins/paint
templates_LTLIBRARIES = templates.la

templates_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

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

gladedir = $(pkgdatadir)/glade/paint/plugins/templates
dist_glade_DATA = \
	templates.glade \
	new-template.glade
