if WITH_OPENBABEL

lib_LTLIBRARIES= libgtkchem3dviewer.la

libgtkchem3dviewer_la_SOURCES = \
		gtkchem3dviewer.cc

libgtkchem3dviewer_la_headers = \
		gtkchem3dviewer.h

gcuincludedir = $(includedir)/gcu/chem3d
gcuinclude_HEADERS = $(libgtkchem3dviewer_la_headers)
		
libgtkchem3dviewer_la_LDFLAGS = -version-info  @GTKCV_VERSION_INFO@ @GCU_LIBS@ \
				@GC3D_LIBS@ @OPENBABEL_LIBS@
AM_CXXFLAGS = @GCU_CFLAGS@ @GC3D_CFLAGS@ @OPENBABEL_INC@

endif
