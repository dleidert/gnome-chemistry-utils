EXTRA_DIST = \
	$(desktop_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) \
	$(schemas_DATA)

MAINTAINERCLEANFILES = \
	Makefile.in

desktop_in_files = \
	gchempaint-unstable.desktop.in \
	x-gchempaint.desktop.in
schemas_in_files = gchempaint.schemas.in

desktopdir = $(datadir)/applications
desktop_DATA = gchempaint-unstable.desktop

schemasdir = $(sysconfdir)/gconf/schemas
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gchempaint.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
CLEANFILES += kde_mime_desktop_DATA
endif #WITH_KDE_MIME

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_SCHEMAS_RULE@

bin_PROGRAMS = gchempaint-unstable

gchempaint_unstable_SOURCES = \
						main.cc \
						standaloneapp.cc	\
						standalonewin.cc

noinst_HEADERS = \
					standaloneapp.h \
					standalonewin.h

gchempaint_unstable_LDADD = \
	$(top_builddir)/libs/gcp/libgchempaint-unstable.la \
	@GCU_LIBS@ 

AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/paint\"
