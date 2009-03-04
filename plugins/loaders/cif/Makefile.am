EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_cifdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/cif
gcu_loader_cif_LTLIBRARIES = cif.la 

cif_la_LDFLAGS = -module -avoid-version -no-undefined

cif_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

cif_la_SOURCES =	\
	cif.cc

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(gcu_loader_cifdir)
@INTLTOOL_XML_RULE@
