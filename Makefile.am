SUBDIRS = po database glade pixmaps gcu programs mozilla-plugin \
	samples tests dtds docs

mimedir = $(datadir)/mime/packages
mime_in_files = gchemutils.xml.in
mime_DATA = $(mime_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST=	\
	gcu-unstable.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in	\
	gnome-doc-utils.make \
	COPYING-DOCS	\
	$(mime_in_files)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu-unstable.pc

CLEANFILES = \
	gchemutils.xml

DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper --disable-update-databases

install-data-hook:
if UPDATE_DATABASES
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif
endif

uninstall-hook:
if UPDATE_DATABASES
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif
endif

distuninstallcheck:
             @:

distcleancheck_listfiles = \
       find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'
