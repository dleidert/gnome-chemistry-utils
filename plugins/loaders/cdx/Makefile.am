EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cdxdir = $(pkglibdir)/plugins/cdx
gcu_loader_cdx_LTLIBRARIES = cdx.la 

cdx_la_LDFLAGS = -module -avoid-version -no-undefined

cdx_la_SOURCES =	\
	cdx.cc

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(gcu_loader_cdxdir)
@INTLTOOL_XML_RULE@
