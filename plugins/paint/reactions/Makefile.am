MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

reactionsdir = $(pkglibdir)/plugins
reactions_LTLIBRARIES = reactions.la

reactions_la_LDFLAGS = @GCU_LIBS@ -module -avoid-version

reactions_la_SOURCES = \
	plugin.cc

noinst_HEADERS = \
	plugin.h

