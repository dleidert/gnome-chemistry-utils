EXTRA_DIST = $(IMAGES)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(textdir)\"

IMAGES = \
	fragment24.png \
	text24.png

VARIABLES = \
	gcp_fragment_24		$(srcdir)/fragment24.png \
	gcp_text_24		$(srcdir)/text24.png

textdir = $(libdir)/gchemutils-unstable/plugins/paint
text_LTLIBRARIES = text.la

text_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

text_la_SOURCES = \
	plugin.cc \
	fragmenttool.cc \
	texttool.cc

noinst_HEADERS = \
	plugin.h \
	fragmenttool.h \
	texttool.h

uidir = $(pkgdatadir)/ui
dist_ui_DATA = fontsel.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/text
glade_DATA = fontsel.glade


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@