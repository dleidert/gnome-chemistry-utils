IMAGES =			\
	bond24.png		\
	chain24.png		\
	upbond24.png	\
	downbond24.png	\
	xbond24.png		\
	forebond24.png	\
	delocalized24.png

VARIABLES = 			 								\
	gcp_bond_24				$(srcdir)/bond24.png		\
	gcp_chain_24			$(srcdir)/chain24.png		\
	gcp_upbond_24			$(srcdir)/upbond24.png		\
	gcp_downbond_24			$(srcdir)/downbond24.png	\
	gcp_xbond_24			$(srcdir)/xbond24.png		\
	gcp_forebond_24			$(srcdir)/forebond24.png	\
	gcp_delocalizedbond_24	$(srcdir)/delocalized24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

bonds_LTLIBRARIES = bonds.la

bondsdir = $(libdir)/gchemutils-unstable/plugins/paint

plugin.$(OBJEXT): $(pixbuf_file)

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

glade_DATA = \
	bond.glade \
	chain.glade

gladedir = $(datadir)/gchemutils-unstable/glade/paint/plugins/bonds

EXTRA_DIST= $(glade_DATA) $(IMAGES)

bonds_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"
