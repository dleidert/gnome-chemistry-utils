IMAGES =			\
	text24.png		\
	fragment24.png

VARIABLES = 			 								\
	gcp_text_24				$(srcdir)/text24.png		\
	gcp_fragment_24			$(srcdir)/fragment24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

text_LTLIBRARIES = text.la

textdir = $(libdir)/gchemutils-unstable/plugins/paint

text_la_SOURCES = \
	plugin.cc \
	texttool.cc \
	fragmenttool.cc

noinst_HEADERS = \
	plugin.h \
	texttool.h \
	fragmenttool.h

glade_DATA = \
	fontsel.glade

gladedir = $(datadir)/gchempaint-unstable/ui

EXTRA_DIST =  $(ui_DATA) $(IMAGES)

text_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"
