EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gspectrum.desktop

MAINTAINERCLEANFILES = Makefile.in gspectrum.desktop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
	-DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui\"

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
	cd $(DESTDIR)$(bindir) && \
	if [ -x gspectrum@STABILITY@ ]; then \
		rm -f gspectrum@STABILITY@; \
	fi && \
	$(LN_S) gspectrum-@GCU_API_VER@ gspectrum@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gspectrum@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gspectrum@STABILITY@; \
	fi
