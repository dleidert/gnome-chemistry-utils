EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemcalc.desktop

MAINTAINERCLEANFILES = Makefile.in gchemcalc.dektop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DGLADEDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/glade\"

bin_PROGRAMS = gchemcalc-@GCU_API_VER@

gchemcalc_@GCU_API_VER@_SOURCES = \
		gchemcalc.cc

desktop_in_files = gchemcalc.desktop.in

gchemcalc-@GCU_API_VER@.desktop: gchemcalc.desktop
	cp $< $@

desktopdir=$(datadir)/applications
desktop_DATA = gchemcalc-@GCU_API_VER@.desktop
@INTLTOOL_DESKTOP_RULE@

install-exec-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gchemcalc ]; then \
		rm $(bindir)/gchemcalc; \
	fi
	$(LN_S) $(bindir)/gchemcalc-@GCU_API_VER@ $(bindir)/gchemcalc
else
	if [ -x $(bindir)/gchemcalc-unstable ]; then \
		rm $(bindir)/gchemcalc-unstable; \
	fi
	$(LN_S) $(bindir)/gchemcalc-@GCU_API_VER@ $(bindir)/gchemcalc-unstable
endif

uninstall-hook:
if GCU_IS_STABLE
	if [ -x $(bindir)/gchemcalc ]; then \
		rm $(bindir)/gchemcalc; \
	fi
else
	if [ -x $(bindir)/gchemcalc-unstable ]; then \
		rm $(bindir)/gchemcalc-unstable; \
	fi
endif
