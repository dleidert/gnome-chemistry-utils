IMAGES =			\
	favicon.png

VARIABLES = 			 								\
	favicon				$(srcdir)/favicon.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

wikipedia_LTLIBRARIES = wikipedia.la

wikipediadir = $(libdir)/gchemutils-unstable/plugins/paint

wikipedia_la_SOURCES = \
	plugin.cc \
	wikipediatool.cc

noinst_HEADERS = \
	plugin.h \
	wikipediatool.h

EXTRA_DIST =  $(ui_DATA) $(IMAGES)

wikipedia_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"

MAINTAINERCLEANFILES = Makefile.in
