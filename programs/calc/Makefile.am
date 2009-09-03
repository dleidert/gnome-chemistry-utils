EXTRA_DIST = $(desktop_in_files)

CLEANFILES = $(desktop_DATA) gchemcalc.desktop

MAINTAINERCLEANFILES = Makefile.in gchemcalc.dektop.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DDATADIR=\"$(datadir)\" \
        -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
        -DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui\"

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
	cd $(DESTDIR)$(bindir) && \
	if [ -x gchemcalc@STABILITY@ ]; then \
		rm -f gchemcalc@STABILITY@; \
	fi && \
	$(LN_S) gchemcalc-@GCU_API_VER@ gchemcalc@STABILITY@

uninstall-hook:
	if [ -x $(DESTDIR)/$(bindir)/gchemcalc@STABILITY@ ]; then \
		rm -f $(DESTDIR)/$(bindir)/gchemcalc@STABILITY@; \
	fi
