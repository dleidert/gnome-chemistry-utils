lib_LTLIBRARIES= libgcu-0.5.la

libgcu_0_5_la_SOURCES = \
		atom.cc \
		bond.cc \
		object.cc \
		document.cc \
		element.cc \
		isotope.cc \
		formula.cc \
		matrix.cc \
		matrix2d.cc \
		xml-utils.cc \
		application.cc \
		dialog.cc \
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
		isotope.h \
		formula.h \
		application.h \
		dialog.h \
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

gcuincludedir = $(includedir)/gcu-0.5/gcu
gcuinclude_HEADERS = $(libgcu_0_5_la_headers)
		
libgcu_0_5_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
