EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemtable.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DGLADEDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/glade/table\"

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
if GCU_IS_STABLE
	if [ -x $(bindir)/gchemtable ]; then \
		rm $(bindir)/gchemtable; \
	fi
	ln -sf $(bindir)/gchemtable-@GCU_API_VER@ $(bindir)/gchemtable
else
	if [ -x $(bindir)/gchemtable-unstable ]; then \
		rm $(bindir)/gchemtable-unstable; \
	fi
	ln -sf $(bindir)/gchemtable-@GCU_API_VER@ $(bindir)/gchemtable-unstable
endif

uninstall-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gchemtable ]; then \
		rm $(bindir)/gchemtable; \
	fi
else
	if [ -x $(bindir)/gchemtable-unstable ]; then \
		rm $(bindir)/gchemtable-unstable; \
	fi
endif
