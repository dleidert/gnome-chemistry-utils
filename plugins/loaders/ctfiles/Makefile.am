EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(gcu_loader_ctfiles_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\"

gcu_loader_ctfilesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/ctfiles
gcu_loader_ctfiles_DATA = $(xml_in_files:.xml.in=.xml)
gcu_loader_ctfiles_LTLIBRARIES = ctfiles.la 

ctfiles_la_LDFLAGS = -module -avoid-version -no-undefined

ctfiles_la_LIBADD = \
		$(gsf_LIBS) $(goffice_LIBS) \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la

ctfiles_la_SOURCES =	\
	ctfiles.cc

xml_in_files = plugin.xml.in

@INTLTOOL_XML_RULE@
