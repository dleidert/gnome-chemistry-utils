EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gspectrum.desktop

MAINTAINERCLEANFILES = Makefile.in gspectrum.desktop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
	-DGLADEDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/glade\"

bin_PROGRAMS = gspectrum-@GCU_API_VER@

gspectrum_@GCU_API_VER@_SOURCES = \
		application.cc \
		application.h \
		document.cc \
		document.h \
		view.cc \
		view.h \
		window.cc \
		window.h \
		gspectrum.cc

desktop_in_files = gspectrum.desktop.in

gspectrum-@GCU_API_VER@.desktop: gspectrum.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gspectrum-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

install-exec-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gspectrum ]; then \
		rm $(bindir)/gspectrum; \
	fi
	$(LN_S) $(bindir)/gspectrum-@GCU_API_VER@ $(bindir)/gspectrum
else
	if [ -x $(bindir)/gspectrum-unstable ]; then \
		rm $(bindir)/gspectrum-unstable; \
	fi
	$(LN_S) $(bindir)/gspectrum-@GCU_API_VER@ $(bindir)/gspectrum-unstable
endif

uninstall-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gspectrum ]; then \
		rm $(bindir)/gspectrum; \
	fi
else
	if [ -x $(bindir)/gspectrum-unstable ]; then \
		rm $(bindir)/gspectrum-unstable; \
	fi
endif
