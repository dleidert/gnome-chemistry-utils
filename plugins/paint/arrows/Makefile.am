EXTRA_DIST = \
	$(IMAGES) \
	org.gnome.gchemutils.paint.plugins.arrows.gschema.xml.in.in

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES) $(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
		-DPLUGINSDIR=\"$(arrowsdir)\"

IMAGES = \
	arrow1-24.png \
	arrow2-24.png \
	curved1-24.png \
	curved24.png \
	mesomery24.png \
	retrosynth24.png

VARIABLES = \
	gcp_arrow1_24			$(srcdir)/arrow1-24.png \
	gcp_arrow2_24			$(srcdir)/arrow2-24.png \
	gcp_retrosynth_24		$(srcdir)/retrosynth24.png \
	gcp_mesomery_24			$(srcdir)/mesomery24.png \
	gcp_curved_24			$(srcdir)/curved24.png \
	gcp_curved1_24			$(srcdir)/curved1-24.png

arrowsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
arrows_LTLIBRARIES = arrows.la

arrows_la_LDFLAGS = -module -avoid-version
arrows_la_LIBADD = \
		$(gtk_LIBS) $(goffice_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

arrows_la_SOURCES = \
	plugin.cc \
	arrowtool.cc \
	curvedarrowtool.cc \
	retrosynthesis.cc \
	retrosynthesisarrow.cc \
	retrosynthesisstep.cc

noinst_HEADERS = \
	plugin.h \
	arrowtool.h \
	curvedarrowtool.h \
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
