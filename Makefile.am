SUBDIRS = po database glade gcu programs mozilla-plugin tests dtds docs

EXTRA_DIST = \
	gcu.pc.in \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in

DISTCLEANFILES = \
	gcu.pc \
	intltool-extract \
	intltool-update \
	intltool-merge

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	config.guess \
	config.sub \
	config.h.in \
	configure \
	install-sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	ltmain.sh \
	missing \
	mkinstalldirs \
	Makefile.in

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --disable-update-databases

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc

install-data-hook:
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif #WITH_UPDATE_DESKTOP

uninstall-hook:
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif #WITH_UPDATE_DESKTOP

distcleancheck_listfiles = \
	find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'

