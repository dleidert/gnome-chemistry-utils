if WITH_OPENBABEL

lib_LTLIBRARIES= libgtkchem3dviewer.la

libgtkchem3dviewer_la_SOURCES = \
		gtkchem3dviewer.cc

libgtkchem3dviewer_la_headers = \
		gtkchem3dviewer.h

gcuincludedir = $(includedir)/gcu/chem3d
gcuinclude_HEADERS = $(libgtkchem3dviewer_la_headers)
		
libgtkchem3dviewer_la_LDFLAGS = -version-info  @GTKC3D_VERSION_INFO@ @GCU_LIBS@ \
				@GC3D_LIBS@
AM_CXXFLAGS = @GCU_CFLAGS@ @GC3D_CFLAGS@
AM_CFLAGS = @GCU_CFLAGS@ @GC3D_CFLAGS@

bin_PROGRAMS = gchem3d gchem3d-viewer

gchem3d_SOURCES = bonobo-control.cc

gchem3d_LDADD = libgtkchem3dviewer.la $(top_builddir)/chemistry/libgcu.la \
				@GCU_LIBS@ @GC3D_LIBS@

gchem3d_viewer_SOURCES = gchem3d-viewer.c

gchem3d_viewer_LDADD = libgtkchem3dviewer.la $(top_builddir)/chemistry/libgcu.la \
				@GCU_LIBS@ @GC3D_LIBS@

serverdir=$(libdir)/bonobo/servers
server_in_files = gchem3d.server.in
server_DATA=gchem3d.server
@INTLTOOL_SERVER_RULE@

endif
