IMAGES =				\
	arrow1-24.png		\
	arrow2-24.png		\
	retrosynth24.png	\
	mesomery24.png		\
	curved24.png		\
	curved1-24.png

VARIABLES = 			 								\
	gcp_arrow1_24			$(srcdir)/arrow1-24.png		\
	gcp_arrow2_24			$(srcdir)/arrow2-24.png		\
	gcp_retrosynth_24		$(srcdir)/retrosynth24.png	\
	gcp_mesomery_24			$(srcdir)/mesomery24.png		\
	gcp_curved_24			$(srcdir)/curved24.png	\
	gcp_curved1_24			$(srcdir)/curved1-24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

arrows_LTLIBRARIES = arrows.la

arrowsdir = $(libdir)/gchemutils-unstable/plugins/paint

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

glade_DATA = \
	arrowtool.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/arrows

schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = gchempaint-arrows.schemas.in
schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

CLEANFILES = $(noinst_DATA) $(schemas_DATA)

EXTRA_DIST = \
	$(ui_DATA)	\
	$(IMAGES)	\
	$(schemas_in_files)

arrows_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"

if GCONF_SCHEMAS_INSTALL
# don't do this if we are building in eg. rpm
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p; \
		done \
	fi
else
install-data-local:
endif
