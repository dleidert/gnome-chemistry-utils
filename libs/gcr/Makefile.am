MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgcrystal-@GCU_API_VER@.la

noinst_HEADERS = \
	application.h \
	atom.h \
	atomsdlg.h \
	bond.h \
	celldlg.h \
	cleavage.h \
	cleavagesdlg.h \
	docprop.h \
	document.h \
	gcrcrystalviewer.h \
	globals.h \
	grid.h \
	line.h \
	linesdlg.h \
	prefs.h \
	sizedlg.h \
	view.h \
	view-settings.h \
	window.h

libgcrystal_@GCU_API_VER@_la_SOURCES =	\
	application.cc \
	atom.cc \
	atomsdlg.cc \
	bond.cc \
	celldlg.cc \
	cleavage.cc \
	cleavagesdlg.cc \
	docprop.cc \
	document.cc \
	gcrcrystalviewer.cc \
	grid.cc \
	line.cc \
	linesdlg.cc \
	prefs.cc \
	sizedlg.cc \
	view.cc \
	view-settings.cc \
	window.cc

libgcrystal_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcrystal_@GCU_API_VER@_la_LIBADD = \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gtk_LIBS) \
		$(xml_LIBS) \
		-lGL
AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		-I$(top_builddir)/libs $(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/crystal\" \
        -DPIXMAPSDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/pixmaps\"
