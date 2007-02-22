EXTRA_DIST = $(desktop_in_files) $(schemas_in_files)

CLEANFILES = $(desktop_DATA) $(schemas_DATA)

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = gcrystal

gcrystal_SOURCES = main.cc\
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

gcrystal_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

DEPS = $(top_builddir)/gcu/libgcu.la
INCLUDES = -I$(top_srcdir) @GCU_CFLAGS@
LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils\" -DGLADEDIR=\"$(datadir)/gchemutils/glade/crystal\"

schemasdir = $(sysconfdir)/gconf/schemas
schemas_in_files = gcrystal.schemas.in
schemas_DATA = $(schemas_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

desktopdir=$(datadir)/applications
desktop_in_files = gcrystal.desktop.in
desktop_DATA = gcrystal.desktop
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      $(GCONFTOOL) --makefile-install-rule gcrystal.schemas
