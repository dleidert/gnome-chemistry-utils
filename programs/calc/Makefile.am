EXTRA_DIST = $(desktop_in_files) gchemcalc.appdata.xml.in

CLEANFILES = $(desktop_DATA) gchemcalc.desktop gchemcalc.appdata.xml

MAINTAINERCLEANFILES = Makefile.in gchemcalc.dektop.in

DEPS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(goffice_LIBS) \
	$(gtk_LIBS) \
	$(gsf_LIBS) \
	$(xml_LIBS) \
	-lGL
AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/calc\"

bin_PROGRAMS = gchemcalc-@GCU_API_VER@

gchemcalc_@GCU_API_VER@_SOURCES = \
		gchemcalc.cc

desktop_in_files = gchemcalc.desktop.in

gchemcalc-@GCU_API_VER@.desktop: gchemcalc.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gchemcalc-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = gchemcalc.appdata.xml
gchemcalc.appdata.xml: gchemcalc.appdata.xml.in
	cp $< $@

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchemcalc@STABILITY@ ]; then \
		rm -f gchemcalc@STABILITY@; \
	fi && \
	$(LN_S) gchemcalc-@GCU_API_VER@ gchemcalc@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchemcalc@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchemcalc@STABILITY@; \
	fi
