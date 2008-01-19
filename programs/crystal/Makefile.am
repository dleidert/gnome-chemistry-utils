EXTRA_DIST = $(desktop_in_files) $(schemas_in_files)

CLEANFILES = $(desktop_DATA) \
	$(schemas_DATA) \
	$(kde_mime_desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -I$(top_srcdir)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(datadir)/gchemutils/glade/crystal\" \
        -DPIXMAPSDIR=\"$(datadir)/gchemutils/pixmaps\"

bin_PROGRAMS = gcrystal

gcrystal_LDADD = $(top_builddir)/gcu/libgcu.la

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
desktopdir=$(datadir)/applications
desktop_DATA = gcrystal.desktop
@INTLTOOL_DESKTOP_RULE@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gcrystal.desktop
kde_mime_desktopdir = $(with_kde_mime_dir)/application
endif #WITH_KDE_MIME

schemas_in_files = gcrystal.schemas.in
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
