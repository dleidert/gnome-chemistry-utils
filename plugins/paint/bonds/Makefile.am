EXTRA_DIST = $(IMAGES)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(gladedir)\" \
        -DPLUGINSDIR=\"$(bondsdir)\"

IMAGES = \
	bond24.png \
	chain24.png \
	delocalized24.png \
	downbond24.png \
	forebond24.png \
	upbond24.png \
	xbond24.png

VARIABLES = \
	gcp_bond_24			$(srcdir)/bond24.png \
	gcp_chain_24			$(srcdir)/chain24.png \
	gcp_delocalizedbond_24		$(srcdir)/delocalized24.png \
	gcp_downbond_24			$(srcdir)/downbond24.png \
	gcp_forebond_24			$(srcdir)/forebond24.png \
	gcp_upbond_24			$(srcdir)/upbond24.png \
	gcp_xbond_24			$(srcdir)/xbond24.png

bondsdir = $(libdir)/gchemutils-unstable/plugins/paint
bonds_LTLIBRARIES = bonds.la

bonds_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

bonds_la_SOURCES = \
	plugin.cc \
	bondtool.cc \
	chaintool.cc \
	delocalizedtool.cc

noinst_HEADERS = \
	plugin.h \
	bondtool.h \
	chaintool.h \
	delocalizedtool.h

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/bonds
dist_glade_DATA = \
	bond.glade \
	chain.glade


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@