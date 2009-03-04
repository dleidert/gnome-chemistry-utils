EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@openbabel_CFLAGS@ \
		@GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cdxdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/cdx
gcu_loader_cdx_LTLIBRARIES = cdx.la 

cdx_la_LDFLAGS = -module -avoid-version -no-undefined

cdx_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

cdx_la_SOURCES =	\
	cdx.cc

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(gcu_loader_cdxdir)
@INTLTOOL_XML_RULE@
