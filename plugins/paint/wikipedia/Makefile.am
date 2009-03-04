EXTRA_DIST = $(IMAGES)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@openbabel_CFLAGS@ \
		@GCU_CFLAGS@

DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(wikipediadir)\"

IMAGES = favicon.png

VARIABLES = \
	favicon		$(srcdir)/favicon.png

wikipediadir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
wikipedia_LTLIBRARIES = wikipedia.la

wikipedia_la_LDFLAGS = -module -avoid-version
wikipedia_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

wikipedia_la_SOURCES = \
	plugin.cc \
	wikipediatool.cc

noinst_HEADERS = \
	plugin.h \
	wikipediatool.h


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
