MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(selectiondir)\"

selectiondir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
selection_LTLIBRARIES = selection.la

selection_la_LDFLAGS = -module -avoid-version
selection_la_LIBADD = \
		$(gtk_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

selection_la_SOURCES = \
	plugin.cc \
	bracketstool.cc \
	erasertool.cc \
	group.cc \
	groupdlg.cc \
	lassotool.cc \
	selectiontool.cc

noinst_HEADERS = \
	plugin.h \
	bracketstool.h \
	erasertool.h \
	group.h \
	groupdlg.h \
	lassotool.h \
	selectiontool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/selection
dist_ui_DATA =	\
	brackets.ui	\
	group.ui
