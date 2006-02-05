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
				   view-settings.cc

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
				 view-settings.h

gcrystal_unstable_LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@

DEPS = \
	$(top_builddir)/gcu/libgcu-unstable.la

INCLUDES = @GCU_CFLAGS@

desktopdir=$(datadir)/applications
desktop_in_files = \
	gcrystal-unstable.desktop.in
desktop_DATA = \
	gcrystal-unstable.desktop
@INTLTOOL_DESKTOP_RULE@

CLEANFILES = \
	gcrystal-unstable.desktop
		
EXTRA_DIST=$(desktop_in_files)

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
