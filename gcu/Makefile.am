lib_LTLIBRARIES= libgcu.la

libgcu_la_SOURCES = \
		atom.cc \
		bond.cc \
		object.cc \
		document.cc \
		element.cc \
		matrix.cc \
		matrix2d.cc \
		xml-utils.cc \
		chemistry.cc \
		gtkperiodic.c

libgcu_la_headers = \
		atom.h \
		bond.h \
		object.h \
		document.h \
		element.h \
		matrix.h \
		matrix2d.h \
		xml-utils.h \
		chemistry.h \
		gtkperiodic.h

if WITH_GTKGL

libgcu_la_SOURCES += \
		gtkcrystalviewer.cc \
		crystalview.cc \
		crystaldoc.cc \
		crystalatom.cc \
		crystalbond.cc \
		crystalline.cc \
		crystalcleavage.cc

libgcu_la_headers += \
		gtkcrystalviewer.h \
		crystalview.h \
		crystaldoc.h \
		crystalatom.h \
		crystalbond.h \
		crystalline.h \
		crystalcleavage.h
		
if WITH_OPENBABEL
libgcu_la_SOURCES += \
		gtkchem3dviewer.cc

libgcu_la_headers += \
		gtkchem3dviewer.h

bin_PROGRAMS = gchem3d gchem3d-viewer

gchem3d_SOURCES = bonobo-control.cc

gchem3d_LDADD = libgcu.la @GCU_LIBS@

gchem3d_viewer_SOURCES = gchem3d-viewer.c

gchem3d_viewer_LDADD = libgcu.la @GCU_LIBS@

serverdir=$(libdir)/bonobo/servers
server_in_files = gchem3d.server.in
server_DATA=gchem3d.server
@INTLTOOL_SERVER_RULE@

desktopdir=$(datadir)/applications
desktop_in_files = gchem3d-viewer.desktop.in
desktop_DATA=gchem3d-viewer.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = gchem3d.server gchem3d-viewer.desktop elements.xml

endif #WITH_OPENBABEL
endif #WITH_GTKGL
		
gcuincludedir = $(includedir)/gcu/gcu
gcuinclude_HEADERS = $(libgcu_la_headers)
		
libgcu_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@

xmldir = $(datadir)/gchemutils

xml_in_files = elements.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(xml_DATA) $(server_DATA) $(desktop_DATA)
