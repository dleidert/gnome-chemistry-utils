EXTRA_DIST = \
	$(desktop_in_files) \
	$(kde_mime_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) gchempaint.desktop \
	$(schemas_DATA) gchempaint.schemas \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcp/libgchempaint-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(pkgdatadir)/glade/paint\"

bin_PROGRAMS = gchempaint

gchempaint_SOURCES = \
		main.cc \
		standaloneapp.cc \
		standaloneapp.h \
		standalonewin.cc \
		standalonewin.h

desktop_in_files = gchempaint.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

kde_mime_in_files = x-gchempaint.desktop.in

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gchempaint.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

schemas_in_files = gchempaint.schemas.in
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(shell echo "$(schemas_in_files)" | sed 's/.schemas.in$$//;$(transform);s/$$/.schemas/')

@INTLTOOL_SCHEMAS_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif

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
