lib_LTLIBRARIES= libgcu-0.5.la

libgcu_0_5_la_SOURCES = \
		atom.cc \
		bond.cc \
		object.cc \
		document.cc \
		element.cc \
		formula.cc \
		matrix.cc \
		matrix2d.cc \
		xml-utils.cc \
		chemistry.cc \
		gtkperiodic.c \
		gtkcrystalviewer.cc \
		crystalview.cc \
		crystaldoc.cc \
		crystalatom.cc \
		crystalbond.cc \
		crystalline.cc \
		crystalcleavage.cc \
		gtkchem3dviewer.cc

libgcu_0_5_la_headers = \
		atom.h \
		bond.h \
		object.h \
		document.h \
		element.h \
		formula.h \
		matrix.h \
		matrix2d.h \
		xml-utils.h \
		chemistry.h \
		gtkperiodic.h \
		gtkcrystalviewer.h \
		crystalview.h \
		crystaldoc.h \
		crystalatom.h \
		crystalbond.h \
		crystalline.h \
		crystalcleavage.h \
		gtkchem3dviewer.h

bin_PROGRAMS = gchem3d-viewer-0.5

gchem3d_viewer_0_5_SOURCES = gchem3d-viewer.c

gchem3d_viewer_0_5_LDADD = libgcu-0.5.la @GCU_LIBS@

desktopdir=$(datadir)/applications
desktop_in_files = gchem3d-viewer-0.5.desktop.in
desktop_DATA=gchem3d-viewer-0.5.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = gchem3d-viewer-0.5.desktop elements.xml
		
gcuincludedir = $(includedir)/gcu-0.5/gcu
gcuinclude_HEADERS = $(libgcu_0_5_la_headers)
		
libgcu_0_5_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@

xmldir = $(datadir)/gchemutils

xml_in_files = elements.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(desktop_in_files)
