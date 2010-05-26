EXTRA_DIST= $(IMAGES)

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
        -DPLUGINSDIR=\"$(atomsdir)\"

IMAGES = \
	charge-24.png \
	charge+24.png \
	electronpair24.png \
	element24.png \
	unpairedelectron24.png \
	orbit24.png

VARIABLES = \
	gcp_chargem_24			$(srcdir)/charge-24.png \
	gcp_chargep_24			$(srcdir)/charge+24.png \
	gcp_elecpair_24			$(srcdir)/electronpair24.png \
	gcp_element_24			$(srcdir)/element24.png \
	gcp_unpairedelec_24		$(srcdir)/unpairedelectron24.png \
	gcp_orbit_24			$(srcdir)/orbit24.png

atomsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
atoms_LTLIBRARIES = atoms.la

atoms_la_LDFLAGS = -module -avoid-version
atoms_la_LIBADD = \
		$(gtk_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

atoms_la_SOURCES = \
	plugin.cc \
	chargetool.cc \
	electrontool.cc \
	elementtool.cc \
	orbital.cc \
	orbitaltool.cc

noinst_HEADERS = \
	plugin.h \
	chargetool.h \
	electrontool.h \
	elementtool.h \
	orbital.h \
	orbitaltool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/atoms
dist_ui_DATA = \
	orbital.ui \
	orbital-prop.ui


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
