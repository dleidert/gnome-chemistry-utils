EXTRA_DIST = $(desktop_in_files) gchem3d.appdata.xml.in

CLEANFILES = $(desktop_DATA) gchem3d.desktop gchem3d.appdata.xml

MAINTAINERCLEANFILES = Makefile.in gchem3d.desktop.in

DEPS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(goffice_LIBS) \
		$(gtk_LIBS) \
		$(gsf_LIBS) \
		-lGL

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gchem3d-@GCU_API_VER@

gchem3d_@GCU_API_VER@_SOURCES = \
		application.cc \
		application.h \
		document.cc \
		document.h \
		main.cc	\
		view.cc	\
		view.h \
		window.cc \
		window.h

desktop_in_files = gchem3d.desktop.in

gchem3d-@GCU_API_VER@.desktop: gchem3d.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gchem3d-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = gchem3d.appdata.xml
gchem3d.appdata.xml: gchem3d.appdata.xml.in
	cp $< $@

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchem3d@STABILITY@ ]; then \
		rm -f gchem3d@STABILITY@; \
	fi && \
	$(LN_S) gchem3d-@GCU_API_VER@ gchem3d@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchem3d@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchem3d@STABILITY@; \
	fi
