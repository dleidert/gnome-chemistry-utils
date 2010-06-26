MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgcr-@GCU_API_VER@.la

noinst_HEADERS = \
	atom.h \
	bond.h \
	cleavage.h \
	document.h \
	gcrcrystalviewer.h \
	line.h \
	view.h \
	window.h

libgcr_@GCU_API_VER@_la_SOURCES =	\
	atom.cc \
	bond.cc \
	cleavage.cc \
	document.cc \
	gcrcrystalviewer.cc \
	line.cc \
	view.cc \
	window.cc

libgcr_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcr_@GCU_API_VER@_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(gtkglext_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\"
