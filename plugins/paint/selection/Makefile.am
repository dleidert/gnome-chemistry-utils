IMAGES =			\
	selection24.png \
	eraser24.png	\
	horiz24.png	\
	vert24.png	\
	rotate24.png	\
	merge24.png

VARIABLES = 			 								\
	gcp_selection_24		$(srcdir)/selection24.png	\
	gcp_eraser_24			$(srcdir)/eraser24.png	\
	gcp_horiz_24			$(srcdir)/horiz24.png	\
	gcp_vert_24				$(srcdir)/vert24.png	\
	gcp_rotate_24			$(srcdir)/rotate24.png	\
	gcp_merge_24			$(srcdir)/merge24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

selection_LTLIBRARIES = selection.la

selectiondir = $(libdir)/gchemutils-unstable/plugins/paint

selection_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/paint/plugins/selection\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"

selection_la_SOURCES = \
	plugin.cc \
	selectiontool.cc \
	erasertool.cc \
	group.cc \
	groupdlg.cc

noinst_HEADERS = \
	plugin.h \
	selectiontool.h \
	erasertool.h \
	group.h \
	groupdlg.h

glade_DATA = \
	group.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/selection

EXTRA_DIST= $(IMAGES) $(glade_DATA)
