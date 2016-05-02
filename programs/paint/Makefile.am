EXTRA_DIST = \
	$(desktop_in_files) \
	gchempaint.appdata.xml.in

CLEANFILES = \
	$(desktop_DATA) gchempaint.desktop \
	$(kde_mime_desktop_DATA) \
	gchempaint.appdata.xml

MAINTAINERCLEANFILES = Makefile.in gchempaint.desktop.in

DEPS = $(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
	$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(goffice_LIBS) \
	$(gtk_LIBS) \
	-lGL
AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/paint\"

bin_PROGRAMS = gchempaint-@GCU_API_VER@

gchempaint_@GCU_API_VER@_SOURCES = \
		main.cc \
		standaloneapp.cc \
		standaloneapp.h \
		standalonewin.cc \
		standalonewin.h

desktop_in_files = gchempaint.desktop.in x-gchempaint.desktop.in

gchempaint-@GCU_API_VER@.desktop: gchempaint.desktop
	cp $< $@

desktopdir = $(datadir)/applications
desktop_DATA = gchempaint-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = gchempaint.appdata.xml
gchempaint.appdata.xml: gchempaint.appdata.xml.in
	cp $< $@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gchempaint.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchempaint@STABILITY@ ]; then \
		rm -f gchempaint@STABILITY@; \
	fi && \
	$(LN_S) gchempaint-@GCU_API_VER@ gchempaint@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchempaint@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchempaint@STABILITY@; \
	fi
