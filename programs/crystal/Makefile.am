EXTRA_DIST = \
	$(desktop_in_files)

CLEANFILES = \
	$(desktop_DATA) gcrystal.desktop \
	$(kde_mime_desktop_DATA) \
	gcrystal.appdata.xml

MAINTAINERCLEANFILES = Makefile.in gcrystal.desktop.in

DEPS = $(top_builddir)/libs/gcr/libgcrystal-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcr/libgcrystal-@GCU_API_VER@.la \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gtk_LIBS) \
		-lGL

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/crystal\"

bin_PROGRAMS = gcrystal-@GCU_API_VER@

gcrystal_@GCU_API_VER@_SOURCES = \
		application.cc \
		application.h \
		atom.cc \
		atom.h \
		cleavage.cc \
		cleavage.h \
		document.cc \
		document.h \
		gcrystal.h \
		line.cc \
		line.h \
		main.cc \
		view.cc \
		view.h \
		window.cc \
		window.h

desktop_in_files = gcrystal.desktop.in x-gcrystal.desktop.in

gcrystal-@GCU_API_VER@.desktop: gcrystal.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gcrystal-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = gcrystal.appdata.xml
gcrystal.appdata.xml: gcrystal.appdata.xml.in
	cp $< $@

if WITH_KDE_MIME
kde_mime_desktop_DATA = x-gcrystal.desktop
kde_mime_desktopdir = $(KDEMIMEDIR)/application
endif #WITH_KDE_MIME

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gcrystal@STABILITY@ ]; then \
		rm -f gcrystal@STABILITY@; \
	fi && \
	$(LN_S) gcrystal-@GCU_API_VER@ gcrystal@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gcrystal@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gcrystal@STABILITY@; \
	fi
