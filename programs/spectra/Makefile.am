EXTRA_DIST = $(desktop_in_files) gspectrum.appdata.xml.in

CLEANFILES = $(desktop_DATA) gspectrum.desktop gspectrum.appdata.xml

MAINTAINERCLEANFILES = Makefile.in gspectrum.desktop.in

DEPS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(goffice_LIBS) \
	$(gsf_LIBS) \
	$(gtk_LIBS) \
	-lGL
AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
		-DLOCALEDIR=\"$(localedir)\" \
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

appdatadir = $(datadir)/appdata
appdata_DATA = gspectrum.appdata.xml
gspectrum.appdata.xml: gspectrum.appdata.xml.in
	cp $< $@

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
