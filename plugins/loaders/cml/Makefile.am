EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(gcu_loader_cml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cmldir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/cml
gcu_loader_cml_DATA = $(xml_in_files:.xml.in=.xml)
gcu_loader_cml_LTLIBRARIES = cml.la 

cml_la_LDFLAGS = -module -avoid-version -no-undefined

cml_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

cml_la_SOURCES =	\
	cml.cc

xml_in_files = plugin.xml.in

@INTLTOOL_XML_RULE@
