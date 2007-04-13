bin_PROGRAMS = gchempaint-unstable

gchempaint_unstable_SOURCES = \
						main.cc \
						standaloneapp.cc	\
						standalonewin.cc

noinst_HEADERS = \
					standaloneapp.h \
					standalonewin.h

gchempaint_unstable_LDADD = \
	$(top_srcdir)/libs/gcp/libgchempaint-unstable.la \
	@GCU_LIBS@ 

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/paint\"
