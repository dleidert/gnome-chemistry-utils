MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(atomsdir)\"

atomsdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
atoms_LTLIBRARIES = atoms.la

atoms_la_LDFLAGS = -module -avoid-version
atoms_la_LIBADD = \
		$(gtk_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
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
