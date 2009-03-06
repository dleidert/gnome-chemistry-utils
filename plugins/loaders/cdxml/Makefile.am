EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cdxmldir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/cdxml
gcu_loader_cdxml_DATA = $(xml_in_files:.xml.in=.xml)
gcu_loader_cdxml_LTLIBRARIES = cdxml.la 

cdxml_la_LDFLAGS = -module -avoid-version -no-undefined

cdxml_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

cdxml_la_SOURCES =	\
	cdxml.cc

xml_in_files = plugin.xml.in

@INTLTOOL_XML_RULE@
