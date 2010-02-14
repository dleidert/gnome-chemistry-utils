EXTRA_DIST = $(IMAGES) $(glade_DATA)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

if GCU_NEEDS_GCONF
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(glade_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(gconf_CFLAGS) \
		$(GCU_CFLAGS)
else
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(gnome_canvas_CFLAGS) \
		$(glade_CFLAGS) \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)
endif

DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(textdir)\"

IMAGES = \
	fragment24.png \
	text24.png

VARIABLES = \
	gcp_fragment_24		$(srcdir)/fragment24.png \
	gcp_text_24		$(srcdir)/text24.png

textdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
text_LTLIBRARIES = text.la

text_la_LDFLAGS = -module -avoid-version
text_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la

text_la_SOURCES = \
	plugin.cc \
	fragmenttool.cc \
	texttool.cc

noinst_HEADERS = \
	plugin.h \
	fragmenttool.h \
	texttool.h

gladedir = $(datadir)/gchemutils/@GCU_API_VER@/glade/paint/plugins/text
glade_DATA = fontsel.glade

$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
