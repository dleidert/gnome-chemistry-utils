EXTRA_DIST = \
	$(IMAGES) \
	$(schemas_in_files)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES) $(schemas_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
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

arrowsdir = $(libdir)/gchemutils/plugins/paint
arrows_LTLIBRARIES = arrows.la

arrows_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

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

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/arrows
dist_glade_DATA = arrowtool.glade

schemas_in_files = gchempaint-arrows.schemas.in

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@

install-data-local:
if GCONF_SCHEMAS_INSTALL
	for p in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-install-rule $$p; \
	done
endif #GCONF_SCHEMAS_INSTALL

uninstall-local:
if GCONF_SCHEMAS_INSTALL
	for schema in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-uninstall-rule $${schema} ; \
	done
endif #GCONF_SCHEMAS_INSTALL