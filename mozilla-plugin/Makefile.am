MAINTAINERCLEANFILES = Makefile.in

if WITH_MOZILLA
plugin_LTLIBRARIES = libmozgcu.la
libexec_PROGRAMS = chem-viewer
plugindir = $(mozlibdir)/plugins/
else
plugin_LTLIBRARIES =
libexec_PROGRAMS =
plugindir =
endif

DEFS += -DLIBEXECDIR=\"$(libexecdir)\"

libmozgcu_la_SOURCES = moz-plugin.c
libmozgcu_la_CPPFLAGS = $(MOZILLA_CFLAGS)
libmozgcu_la_LDFLAGS = -module -avoid-version 
libmozgcu_la_LIBADD = $(MOZILLA_LIBS)

chem_viewer_SOURCES = chem-viewer.cc
chem_viewer_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@
chem_viewer_CPPFLAGS =  @GCU_CFLAGS@
