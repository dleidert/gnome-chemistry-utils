plugindir = $(libdir)/mozilla/plugins/

if WITH_MOZILLA
plugin_LTLIBRARIES = libmozgcu.la
else
plugin_LTLIBRARIES = 
endif

INCLUDES=$(MOZILLA_CFLAGS) -DBINDIR=\"$(bindir)/\"

libmozgcu_la_SOURCES = moz-plugin.c
libmozgcu_la_LDFLAGS = -module -avoid-version 
libmozgcu_la_LIBADD = $(GLIB_LIBS)
EXTRA_DIST=moz-plugin.c
