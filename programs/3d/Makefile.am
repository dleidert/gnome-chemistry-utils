EXTRA_DIST = $(desktop_in_files) 

CLEANFILES = $(desktop_DATA) gchem3d.desktop

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\"

bin_PROGRAMS = gchem3d

gchem3d_SOURCES = \
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
desktopdir = $(datadir)/applications
desktop_DATA = $(shell echo "$(desktop_in_files)" | sed 's/.desktop.in$$//;$(transform);s/$$/.desktop/')

@INTLTOOL_DESKTOP_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif
