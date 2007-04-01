IMAGES =				\
	element24.png		\
	charge-24.png		\
	charge+24.png		\
	electronpair24.png	\
	unpairedelectron24.png

VARIABLES = 			 									\
	gcp_element_24			$(srcdir)/element24.png			\
	gcp_chargem_24			$(srcdir)/charge-24.png			\
	gcp_chargep_24			$(srcdir)/charge+24.png			\
	gcp_elecpair_24			$(srcdir)/electronpair24.png	\
	gcp_unpairedelec_24		$(srcdir)/unpairedelectron24.png

pixbuf_file = gcp-stock-pixbufs.h
noinst_DATA = $(pixbuf_file)
CLEANFILES = $(noinst_DATA)

$(pixbuf_file): Makefile $(IMAGES)
	@echo $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) '>$@'
	@     $(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) >$@

BUILT_SOURCES = $(pixbuf_file)

atoms_LTLIBRARIES = atoms.la

atomsdir = $(libdir)/gchemutils-unstable/plugins/paint

atoms_la_SOURCES = \
	plugin.cc \
	elementtool.cc \
	chargetool.cc \
	electrontool.cc

noinst_HEADERS = \
	plugin.h \
	elementtool.h \
	chargetool.h \
	electrontool.h

EXTRA_DIST= $(IMAGES)

atoms_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\"
