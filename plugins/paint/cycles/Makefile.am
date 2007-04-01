IMAGES =		\
	c3-24.png	\
	c4-24.png	\
	c5-24.png	\
	c6-24.png	\
	c7-24.png	\
	c8-24.png	\
	cn-24.png

VARIABLES = 						\
	gcp_c3_24	$(srcdir)/c3-24.png	\
	gcp_c4_24	$(srcdir)/c4-24.png	\
	gcp_c5_24	$(srcdir)/c5-24.png	\
	gcp_c6_24	$(srcdir)/c6-24.png	\
	gcp_c7_24	$(srcdir)/c7-24.png	\
	gcp_c8_24	$(srcdir)/c8-24.png	\
	gcp_cn_24	$(srcdir)/cn-24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

cycles_LTLIBRARIES = cycles.la

cyclesdir = $(libdir)/gchemutils-unstable/plugins/paint

cycles_la_SOURCES = \
	plugin.cc \
	cycletool.cc

noinst_HEADERS = \
	plugin.h \
	cycletool.h

glade_DATA = \
	cycle.glade \
	cyclen.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/cycles

EXTRA_DIST= $(glade_DATA) $(IMAGES)

cycles_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"
