EXTRA_DIST = $(xml_in_files)

DISTCLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

goffice_gchemutilsdir = $(GOFFICE_PLUGINS_DIR)/gchemutils
goffice_gchemutils_LTLIBRARIES = gchemutils.la

gchemutils_la_LDFLAGS = -module -avoid-version -no-undefined

gchemutils_la_LIBADD = \
	$(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la \
	@GCU_LIBS@

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
