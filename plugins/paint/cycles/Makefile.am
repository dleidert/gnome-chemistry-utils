MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(cyclesdir)\"

cyclesdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
cycles_LTLIBRARIES = cycles.la

cycles_la_LDFLAGS = -module -avoid-version
cycles_la_LIBADD = \
		$(gtk_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

cycles_la_SOURCES = \
	plugin.cc \
	cycletool.cc

noinst_HEADERS = \
	plugin.h \
	cycletool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/cycles
dist_ui_DATA = \
	cycle.ui \
	cyclen.ui
