EXTRA_DIST = $(IMAGES)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(cyclesdir)\"

IMAGES = \
	c3-24.png \
	c4-24.png \
	c5-24.png \
	c6-24.png \
	c7-24.png \
	c8-24.png \
	cn-24.png

VARIABLES = \
	gcp_c3_24	$(srcdir)/c3-24.png \
	gcp_c4_24	$(srcdir)/c4-24.png \
	gcp_c5_24	$(srcdir)/c5-24.png \
	gcp_c6_24	$(srcdir)/c6-24.png \
	gcp_c7_24	$(srcdir)/c7-24.png \
	gcp_c8_24	$(srcdir)/c8-24.png \
	gcp_cn_24	$(srcdir)/cn-24.png

cyclesdir = $(libdir)/gchemutils-unstable/plugins/paint
cycles_LTLIBRARIES = cycles.la

cycles_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

cycles_la_SOURCES = \
	plugin.cc \
	cycletool.cc

noinst_HEADERS = \
	plugin.h \
	cycletool.h

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/cycles
dist_glade_DATA = \
	cycle.glade \
	cyclen.glade


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@