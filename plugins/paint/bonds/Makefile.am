EXTRA_DIST = $(IMAGES)

BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(bondsdir)\"

IMAGES = \
	delocalized24.png \
	downbond24.png \
	idownbond24.png \
	forebond24.png \
	upbond24.png \
	xbond24.png

VARIABLES = \
	gcp_delocalizedbond_24		$(srcdir)/delocalized24.png \
	gcp_downbond_24			$(srcdir)/downbond24.png \
	gcp_idownbond_24		$(srcdir)/idownbond24.png \
	gcp_forebond_24			$(srcdir)/forebond24.png \
	gcp_upbond_24			$(srcdir)/upbond24.png \
	gcp_xbond_24			$(srcdir)/xbond24.png

bondsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
bonds_LTLIBRARIES = bonds.la

bonds_la_LDFLAGS = -module -avoid-version
bonds_la_LIBADD = \
		$(gtk_LIBS) $(goffice_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

bonds_la_SOURCES = \
	plugin.cc \
	bondtool.cc \
	chaintool.cc \
	delocalizedtool.cc \
	newman.cc

noinst_HEADERS = \
	plugin.h \
	bondtool.h \
	chaintool.h \
	delocalizedtool.h \
	newman.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/bonds
dist_ui_DATA = \
	bond.ui \
	chain.ui \
	newman.ui


$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@
