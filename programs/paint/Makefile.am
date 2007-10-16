EXTRA_DIST = \
	$(desktop_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) \
	$(schemas_DATA) \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
LDADD = $(top_builddir)/libs/gcp/libgchempaint-unstable.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/paint\"

bin_PROGRAMS = gchempaint-unstable

gchempaint_unstable_SOURCES = \
		main.cc \
		standaloneapp.cc \
		standaloneapp.h \
		standalonewin.cc \
		standalonewin.h

desktop_in_files = gchempaint-unstable.desktop.in x-gchempaint.desktop.in

desktopdir = $(datadir)/applications
desktop_DATA = gchempaint-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gchempaint.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

schemas_in_files = gchempaint.schemas.in

schemasdir = $(sysconfdir)/gconf/schemas
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

install-data-local:
if GCONF_SCHEMAS_INSTALL
	for schema in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-install-rule $${schema} ; \
	done
endif #GCONF_SCHEMAS_INSTALL

uninstall-local:
if GCONF_SCHEMAS_INSTALL
	for schema in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-uninstall-rule $${schema} ; \
	done
endif #GCONF_SCHEMAS_INSTALL
