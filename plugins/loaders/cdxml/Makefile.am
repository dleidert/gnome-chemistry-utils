EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cdxmldir = $(libdir)/gchemutils/$(VERSION)/cdxml
gcu_loader_cdxml_LTLIBRARIES = cdxml.la 

cdxml_la_LDFLAGS = -module -avoid-version -no-undefined

cdxml_la_LIBADD = \
	$(top_builddir)/libs/gcu/libgcu-unstable.la \
	@GCU_LIBS@

cdxml_la_SOURCES =	\
	cdxml.cc

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(gcu_loader_cdxmldir)
@INTLTOOL_XML_RULE@
