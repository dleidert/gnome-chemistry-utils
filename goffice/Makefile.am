EXTRA_DIST = $(xml_in_files)

DISTCLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEFS += -DDATADIR=\"$(datadir)\"

goffice_gchemutilsdir = $(GOFFICE_PLUGINS_DIR)/gchemutils
goffice_gchemutils_LTLIBRARIES = gchemutils.la

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

gchemutils_la_LDFLAGS = \
		-module -avoid-version -no-undefined

gchemutils_la_LIBADD = \
		$(gtk_LIBS) $(goffice_LIBS) $(gsf_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcr/libgcrystal-@GCU_API_VER@.la

gchemutils_la_SOURCES =	\
		gchemutils.cc	\
		gchemutils.h	\
		gchemutils-priv.h	\
		gogcpapp.cc	\
		gogcpapp.h	\
		gogcpwin.cc	\
		gogcpwin.h	\
		gogcuapp.cc	\
		gogcuapp.h	\
		gogcrystalapp.cc	\
		gogcrystalapp.h	\
		gogcrystalwin.cc	\
		gogcrystalwin.h	\
		gogchem3dapp.cc	\
		gogchem3dapp.h	\
		gogchem3dwin.cc	\
		gogchem3dwin.h	

xml_in_files = plugin.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)
xmldir = $(goffice_gchemutilsdir)

@INTLTOOL_XML_RULE@
