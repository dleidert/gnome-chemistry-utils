EXTRA_DIST = $(desktop_in_files) $(schemas_in_files)

CLEANFILES = $(desktop_DATA) $(schemas_DATA)

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gcrystal-unstable

gcrystal_unstable_SOURCES = main.cc\
				   application.cc\
				   document.cc\
				   atom.cc\
				   line.cc\
				   celldlg.cc \
				   atomsdlg.cc \
				   linesdlg.cc \
				   cleavagesdlg.cc \
				   sizedlg.cc \
				   view.cc\
				   cleavage.cc\
				   prefs.cc \
				   view-settings.cc \
				   window.cc

noinst_HEADERS = gcrystal.h\
				 globals.h\
				 application.h\
				 document.h\
				 atom.h\
				 line.h\
				 atomsdlg.h \
				 celldlg.h \
				 linesdlg.h \
				 cleavagesdlg.h \
				 sizedlg.h \
				 view.h\
				 cleavage.h\
				 prefs.h \
				 view-settings.h \
				 window.h

gcrystal_unstable_LDADD = $(top_builddir)/libs/gcu/libgcu-unstable.la @GCU_LIBS@

DEPS = $(top_builddir)/libs/gcu/libgcu-unstable.la
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" -DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/crystal\"

schemasdir = $(sysconfdir)/gconf/schemas
schemas_in_files = gcrystal.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gcrystal-unstable.desktop.in \
	x-gcrystal.desktop.in
desktop_DATA = gcrystal-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gcrystal.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

install-data-local:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      $(GCONFTOOL) --makefile-install-rule gcrystal.schemas
