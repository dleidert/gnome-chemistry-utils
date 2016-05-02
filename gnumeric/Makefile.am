EXTRA_DIST = plugin.xml.in functions.cc

DISTCLEANFILES = plugin.xml

MAINTAINERCLEANFILES = Makefile.in

if WITH_GNUMERIC

AM_CPPFLAGS = \
	-I$(top_srcdir)	-I$(top_srcdir)/libs	\
	$(gnumeric_CFLAGS) \
	$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" -DGNM_VERSION_FULL=\"$(gnm_version)\"

gchemutils_plugindir = $(GNUMERIC_PLUGINS_DIR)/gchemutils
gchemutils_plugin_LTLIBRARIES = plugin.la 
plugin_la_LDFLAGS = -module -avoid-version
plugin_la_SOURCES =	\
	functions.cc
plugin_la_LIBADD = \
	$(gnumeric_LIBS) \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la


xmldir = $(gchemutils_plugindir)
xml_in_files = plugin.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

endif

