EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(gcu_loader_nuts_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_nutsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/nuts
gcu_loader_nuts_DATA = $(xml_in_files:.xml.in=.xml)
gcu_loader_nuts_LTLIBRARIES = nuts.la 

nuts_la_LDFLAGS = -module -avoid-version -no-undefined

nuts_la_LIBADD = \
		$(gsf_LIBS) $(goffice_LIBS) \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

nuts_la_SOURCES =	\
	nuts.cc

xml_in_files = plugin.xml.in

@INTLTOOL_XML_RULE@
