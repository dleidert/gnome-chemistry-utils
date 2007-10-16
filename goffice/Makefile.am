EXTRA_DIST = $(xml_in_files)

DISTCLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

goffice_gchemutilsdir = $(GOFFICE_PLUGINS_DIR)/gchemutils
goffice_gchemutils_LTLIBRARIES = gchemutils.la

gchemutils_la_LDFLAGS = -module -avoid-version -no-undefined

gchemutils_la_LIBADD = \
	$(top_builddir)/libs/gcp/libgchempaint-unstable.la \
	@GCU_LIBS@

gchemutils_la_SOURCES =	\
		gchemutils.cc	\
		gchemutils.h	\
		gchemutils-priv.h
#		gofficeapp.cc	\
#		gofficeapp.h	\
#		gofficewin.cc	\
#		gofficewin.h

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(goffice_gchemutilsdir)
@INTLTOOL_XML_RULE@
