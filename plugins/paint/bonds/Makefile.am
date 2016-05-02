MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(bondsdir)\"

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
