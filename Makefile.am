SUBDIRS = po database glade gcu programs mozilla-plugin tests dtds docs

EXTRA_DIST=	\
	gcu-0.5.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu-0.5.pc
	
DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --disable-update-databases

install-data-hook:
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif

uninstall-hook:
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif

# remove the following when we don't install mime type anymore
distuninstallcheck:
             @:

distcleancheck_listfiles = \
       find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'
