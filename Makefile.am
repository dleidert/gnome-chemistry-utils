SUBDIRS = po glade gcu dtds docs tests 

EXTRA_DIST=	\
	gchemutils.keys.in \
	gchemutils.mime \
	$(mime_in_files) \
	gcu.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

CLEANFILES = \
	gchemutils.keys \
	gchemutils.xml
	
DISTCLEANFILES = \
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

if GNOME24_SUPPORT
Mimedir = $(datadir)/mime-info
Mime_DATA = gchemutils.keys gchemutils.mime
keys_in_files = gchemutils.keys.in
@INTLTOOL_KEYS_RULE@
endif #GNOME24_SUPPORT

mimedir = $(datadir)/mime/packages
mime_in_files = gchemutils.xml.in
mime_DATA = $(mime_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc

install-data-hook:
if UPDATE_DATABASES
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif #WITH_UPDATE_DESKTOP
endif #UPDATE_DATABASES

uninstall-hook:
if UPDATE_DATABASES
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif #WITH_UPDATE_DESKTOP
endif #UPDATE_DATABASES

distcleancheck_listfiles = \
	find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'

