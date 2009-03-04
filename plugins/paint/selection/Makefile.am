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
        -DPLUGINSDIR=\"$(selectiondir)\"

IMAGES = \
	brackets24.png \
	eraser24.png \
	horiz24.png \
	merge24.png \
	rotate24.png \
	selection24.png \
	vert24.png

VARIABLES = \
	gcp_brackets_24		$(srcdir)/brackets24.png \
	gcp_eraser_24		$(srcdir)/eraser24.png \
	gcp_horiz_24		$(srcdir)/horiz24.png \
	gcp_merge_24		$(srcdir)/merge24.png \
	gcp_rotate_24		$(srcdir)/rotate24.png \
	gcp_selection_24	$(srcdir)/selection24.png \
	gcp_vert_24		$(srcdir)/vert24.png

selectiondir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
selection_LTLIBRARIES = selection.la

selection_la_LDFLAGS = -module -avoid-version
selection_la_LIBADD = \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

selection_la_SOURCES = \
	plugin.cc \
	bracketstool.cc \
	erasertool.cc \
	group.cc \
	groupdlg.cc \
	selectiontool.cc

noinst_HEADERS = \
	plugin.h \
	bracketstool.h \
	erasertool.h \
	group.h \
	groupdlg.h \
	selectiontool.h

gladedir = $(datadir)/gchemutils/@GCU_API_VER@/glade/paint/plugins/selection
dist_glade_DATA = group.glade


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
