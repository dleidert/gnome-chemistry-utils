lib_LTLIBRARIES= libgcu-0.3.la

libgcu_0_3_la_SOURCES = \
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

libgcu_0_3_la_headers = \
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

libgcu_0_3_la_SOURCES += \
		gtkcrystalviewer.cc \
		crystalview.cc \
		crystaldoc.cc \
		crystalatom.cc \
		crystalbond.cc \
		crystalline.cc \
		crystalcleavage.cc

libgcu_0_3_la_headers += \
		gtkcrystalviewer.h \
		crystalview.h \
		crystaldoc.h \
		crystalatom.h \
		crystalbond.h \
		crystalline.h \
		crystalcleavage.h
		
if WITH_OPENBABEL
libgcu_0_3_la_SOURCES += \
		gtkchem3dviewer.cc

libgcu_0_3_la_headers += \
		gtkchem3dviewer.h

bin_PROGRAMS = gchem3d-0.3

gchem3d_0_3_SOURCES = bonobo-control.cc

gchem3d_0_3_LDADD = libgcu-0.3.la @GCU_LIBS@ @GC3D_LIBS@

serverdir=$(libdir)/bonobo/servers
server_in_files = gchem3d-0.3.server.in
server_DATA=gchem3d-0.3.server
@INTLTOOL_SERVER_RULE@

endif #WITH_OPENBABEL
endif #WITH_GTKGL
		
gcuincludedir = $(includedir)/gcu-0.3/gcu
gcuinclude_HEADERS = $(libgcu_0_3_la_headers)
		
libgcu_0_3_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = @GCU_CFLAGS@
AM_CFLAGS = @GCU_CFLAGS@

xmldir = $(datadir)/gchemutils

xml_in_files = elements.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(xml_DATA)
