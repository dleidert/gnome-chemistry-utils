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
        -DUIDIR=\"$(uidir)\" \
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

cyclesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
cycles_LTLIBRARIES = cycles.la

cycles_la_LDFLAGS = -module -avoid-version
cycles_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

cycles_la_SOURCES = \
	plugin.cc \
	cycletool.cc

noinst_HEADERS = \
	plugin.h \
	cycletool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@.ui/paint/plugins/cycles
dist_ui_DATA = \
	cycle.ui \
	cyclen.ui


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
