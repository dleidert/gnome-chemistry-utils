MAINTAINERCLEANFILES = Makefile.in

if WITH_MOZILLA
AM_CPPFLAGS = -I$(top_srcdir)

DEFS += -DLIBEXECDIR=\"$(libexecdir)\"

plugin_LTLIBRARIES = libmozgcu.la
libmozgcu_la_CFLAGS = $(MOZILLA_CFLAGS)
libmozgcu_la_LIBADD = $(MOZILLA_LIBS)
libmozgcu_la_LDFLAGS = -module -avoid-version 
libmozgcu_la_SOURCES = moz-plugin.c
plugindir = $(with_mozilla_libdir)/plugins/


libexec_PROGRAMS = chem-viewer
chem_viewer_LDADD = $(top_builddir)/gcu/libgcu.la
chem_viewer_SOURCES = chem-viewer.cc
endif #WITH_MOZILLA