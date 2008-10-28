EXTRA_DIST = \
	$(desktop_in_files) \
	$(kde_mime_in_files) \
	$(schemas_in_files)

CLEANFILES = \
	$(desktop_DATA) gcrystal.desktop \
	$(schemas_DATA) gcrystal.schemas \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(pkgdatadir)/glade/crystal\" \
        -DPIXMAPSDIR=\"$(pkgdatadir)/pixmaps\"

bin_PROGRAMS = gcrystal

gcrystal_SOURCES = \
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
		cleavagesdlg.cc \
		cleavagesdlg.h \
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

desktop_in_files = gcrystal.desktop.in
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

kde_mime_in_files = x-gcrystal.desktop.in

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gcrystal.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

schemas_in_files = gcrystal.schemas.in
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
