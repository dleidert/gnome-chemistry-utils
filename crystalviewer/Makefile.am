if WITH_GTKGL

lib_LTLIBRARIES= libgtkcrystalviewer.la

libgtkcrystalviewer_la_SOURCES = \
		gtkcrystalviewer.cc \
		crystalview.cc \
		crystaldoc.cc \
		crystalatom.cc \
		crystalline.cc \
		crystalcleavage.cc

libgtkcrystalviewer_la_headers = \
		gtkcrystalviewer.h \
		crystalview.h \
		crystaldoc.h \
		crystalatom.h \
		crystalline.h \
		crystalcleavage.h

gcuincludedir = $(includedir)/gcu/crystalviewer
gcuinclude_HEADERS = $(libgtkcrystalviewer_la_headers)
		
libgtkcrystalviewer_la_LDFLAGS = -version-info  @GTKCV_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@

endif
