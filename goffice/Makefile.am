EXTRA_DIST = $(xml_in_files)

DISTCLEANFILES = $(xml_DATA)

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
DEFS += -DDATADIR=\"$(datadir)\"

goffice_gchemutilsdir = $(GOFFICE_PLUGINS_DIR)/gchemutils
goffice_gchemutils_LTLIBRARIES = gchemutils.la

gchemutils_la_LDFLAGS = -module -avoid-version -no-undefined

gchemutils_la_LIBADD = \
	$(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la

gchemutils_la_SOURCES =	\
		gchemutils.cc	\
		gchemutils.h	\
		gchemutils-priv.h	\
		gogcpapp.cc	\
		gogcpapp.h	\
		gogcpwin.cc	\
		gogcpwin.h	\
		gogcuapp.cc	\
		gogcuapp.h

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(goffice_gchemutilsdir)
@INTLTOOL_XML_RULE@
