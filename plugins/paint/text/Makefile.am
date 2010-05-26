EXTRA_DIST = $(IMAGES) $(ui_DATA)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
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
		$(gtk_LIBS) $(goffice_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

text_la_SOURCES = \
	plugin.cc \
	fragmenttool.cc \
	texttool.cc

noinst_HEADERS = \
	plugin.h \
	fragmenttool.h \
	texttool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/text
ui_DATA = fontsel.ui

$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
