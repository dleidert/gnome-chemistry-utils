SUBDIRS = po database ui pixmaps libs plugins programs mozilla-plugin \
	openbabel gnumeric goffice templates themes samples tests \
	dtds docs schemas

EXTRA_DIST= \
	$(mime_in_files) \
	gnome-doc-utils.make \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in \
	COPYING-DOCS

CLEANFILES = \
	gchemutils.xml

DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge \
	doltcompile

MAINTAINERCLEANFILES = \
	aclocal.m4 \
	compile \
	config.guess \
	config.sub \
	config.h.in \
	configure \
	depcomp \
	gnome-doc-utils.make \
	install-sh \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in \
	ltmain.sh \
	missing \
	mkinstalldirs \
	Makefile.in

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-schemas-install \
	--disable-update-databases \
	--with-mozilla-libdir=\$${libdir}/mozilla \
	--with-goffice-plugins-dir=\$${libdir}/goffice \
	--with-gnumeric-plugins-dir=\$${libdir}/gnumeric
	
mime_in_files = gchemutils.xml.in

mimedir = $(datadir)/mime/packages
mime_DATA = $(mime_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

ACLOCAL_AMFLAGS = -I m4

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
