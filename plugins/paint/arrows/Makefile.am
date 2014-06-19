EXTRA_DIST = \
	$(IMAGES) \
	org.gnome.gchemutils.paint.plugins.arrows.gschema.xml.in.in

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES) $(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
		-DPLUGINSDIR=\"$(arrowsdir)\"

arrowsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
arrows_LTLIBRARIES = arrows.la

arrows_la_LDFLAGS = -module -avoid-version
arrows_la_LIBADD = \
		$(gtk_LIBS) $(goffice_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

arrows_la_SOURCES = \
	plugin.cc \
	arrowtool.cc \
	curvedarrowtool.cc \
	looptool.cc \
	retrosynthesis.cc \
	retrosynthesisarrow.cc \
	retrosynthesisstep.cc

noinst_HEADERS = \
	plugin.h \
	arrowtool.h \
	curvedarrowtool.h \
	looptool.h \
	retrosynthesis.h \
	retrosynthesisarrow.h \
	retrosynthesisstep.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/arrows
dist_ui_DATA = arrowtool.ui curvedarrowtool.ui

$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@

gsettings_SCHEMAS =	\
	org.gnome.gchemutils.paint.plugins.arrows.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@
