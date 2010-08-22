EXTRA_DIST = \
	$(desktop_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) gcrystal.desktop \
	$(schemas_DATA) \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in gcrystal.desktop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcr/libgcr-@GCU_API_VER@.la \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gtk_LIBS) \
		$(gtkglext_LIBS)

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(gtkglext_CFLAGS) \
		$(openbabel_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/crystal\" \
        -DPIXMAPSDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/pixmaps\"

bin_PROGRAMS = gcrystal-@GCU_API_VER@

gcrystal_@GCU_API_VER@_SOURCES = \
		application.cc \
		application.h \
		atom.cc \
		atom.h \
		atomsdlg.cc \
		atomsdlg.h \
		celldlg.cc \
		celldlg.h \
		cleavage.cc \
		cleavage.h \
		docprop.cc \
		docprop.h \
		document.cc \
		document.h \
		gcrystal.h \
		globals.h \
		line.cc \
		line.h \
		linesdlg.cc \
		linesdlg.h \
		main.cc \
		prefs.cc \
		prefs.h \
		sizedlg.cc \
		sizedlg.h \
		view.cc \
		view.h \
		view-settings.cc \
		view-settings.h \
		window.cc \
		window.h

desktop_in_files = gcrystal.desktop.in x-gcrystal.desktop.in

gcrystal-@GCU_API_VER@.desktop: gcrystal.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gcrystal-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gcrystal.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

schemas_in_files = gcrystal.schemas.in

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
	if [ -x gcrystal@STABILITY@ ]; then \
		rm -f gcrystal@STABILITY@; \
	fi && \
	$(LN_S) gcrystal-@GCU_API_VER@ gcrystal@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gcrystal@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gcrystal@STABILITY@; \
	fi
