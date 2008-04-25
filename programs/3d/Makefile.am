EXTRA_DIST = $(desktop_in_files) 

CLEANFILES = $(desktop_DATA) gchem3d.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
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

install-exec-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gchem3d ]; then \
		rm $(bindir)/gchem3d; \
	fi
	ln -sf $(bindir)/gchem3d-@GCU_API_VER@ $(bindir)/gchem3d
else
	if [ -x $(bindir)/gchem3d-unstable ]; then \
		rm $(bindir)/gchem3d-unstable; \
	fi
	ln -sf $(bindir)/gchem3d-@GCU_API_VER@ $(bindir)/gchem3d-unstable
endif

uninstall-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gchem3d ]; then \
		rm $(bindir)/gchem3d; \
	fi
else
	if [ -x $(bindir)/gchem3d-unstable ]; then \
		rm $(bindir)/gchem3d-unstable; \
	fi
endif
