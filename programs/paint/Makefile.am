EXTRA_DIST = \
	$(desktop_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) gchempaint.desktop \
	$(schemas_DATA) \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in gchempaint.desktop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@openbabel_CFLAGS@
		@GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/paint\"

bin_PROGRAMS = gchempaint-@GCU_API_VER@

gchempaint_@GCU_API_VER@_SOURCES = \
		main.cc \
		standaloneapp.cc \
		standaloneapp.h \
		standalonewin.cc \
		standalonewin.h

desktop_in_files = gchempaint.desktop.in x-gchempaint.desktop.in

gchempaint-@GCU_API_VER@.desktop: gchempaint.desktop
	cp $< $@

desktopdir = $(datadir)/applications
desktop_DATA = gchempaint-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gchempaint.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

schemas_in_files = gchempaint.schemas.in

if USE_GCONF
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
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
endif #USE_GCONF

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchempaint@STABILITY@ ]; then \
		rm -f gchempaint@STABILITY@; \
	fi && \
	$(LN_S) gchempaint-@GCU_API_VER@ gchempaint@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchempaint@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchempaint@STABILITY@; \
	fi
