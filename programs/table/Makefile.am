EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemtable.desktop

MAINTAINERCLEANFILES = Makefile.in gchemtable.desktop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la\
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la\
	$(goffice_LIBS) \
	$(gsf_LIBS) \
	$(gtk_LIBS)
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/table\"

bin_PROGRAMS = gchemtable-@GCU_API_VER@

gchemtable_@GCU_API_VER@_SOURCES = \
		gchemtable-main.cc \
		gchemtable-app.cc \
		gchemtable-app.h \
		gchemtable-elt.cc \
		gchemtable-elt.h \
		gchemtable-curve.cc \
		gchemtable-curve.h \
		gchemtable-data-allocator.cc \
		gchemtable-data-allocator.h \
		gchemtable-graph-editor.cc \
		gchemtable-graph-editor.h \
		gchemtable-data.cc \
		gchemtable-data.h

desktop_in_files = gchemtable.desktop.in

gchemtable-@GCU_API_VER@.desktop: gchemtable.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gchemtable-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

install-exec-hook:
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchemtable@STABILITY@ ]; then \
		rm -f gchemtable@STABILITY@; \
	fi && \
	$(LN_S) gchemtable-@GCU_API_VER@ gchemtable@STABILITY@
uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchemtable@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchemtable@STABILITY@; \
	fi
