MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(residuesdir)\"

templatesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
templates_LTLIBRARIES = templates.la

templates_la_LDFLAGS = -module -avoid-version
templates_la_LIBADD = \
		$(gtk_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

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

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/templates
dist_ui_DATA = \
	templates.ui \
	new-template.ui
