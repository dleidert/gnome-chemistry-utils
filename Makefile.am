SUBDIRS = po database glade gcu programs mozilla-plugin tests dtds docs

EXTRA_DIST = \
	gcu.pc.in \
	$(mime_in_files) \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in

CLEANFILES = \
	gchemutils.xml

DISTCLEANFILES = \
	gcu.pc \
	intltool-extract \
	intltool-update \
	intltool-merge

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	compile \
	config.guess \
	config.sub \
	config.h.in \
	configure \
	depcomp \
	install-sh \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	ltmain.sh \
	missing \
	mkinstalldirs \
	Makefile.in

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-update-databases \
	--with-mozilla-libdir=\$${libdir}/mozilla

mime_in_files = gchemutils.xml.in

@INTLTOOL_XML_RULE@

if WITH_UPDATE_MIME
mimedir = $(datadir)/mime/packages
mime_DATA = $(mime_in_files:.xml.in=.xml)
endif #WITH_UPDATE_MIME

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc

install-data-hook:
if WITH_UPDATE_MIME
	$(UPDATE_MIME_DATABASE) $(datadir)/mime
endif
if WITH_UPDATE_DESKTOP
	$(UPDATE_DESKTOP_DATABASE) $(datadir)/applications
endif #WITH_UPDATE_DESKTOP

uninstall-hook:
if WITH_UPDATE_MIME
	$(UPDATE_MIME_DATABASE) $(datadir)/mime
endif #WITH_UPDATE_MIME
if WITH_UPDATE_DESKTOP
	$(UPDATE_DESKTOP_DATABASE) $(datadir)/applications
endif #WITH_UPDATE_DESKTOP

distcleancheck_listfiles = \
	find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'
