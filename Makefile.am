SUBDIRS = po database glade pixmaps libs plugins programs mozilla-plugin \
	goffice templates themes samples tests dtds docs

EXTRA_DIST= \
	$(mime_in_files) \
	gnome-doc-utils.make \
	$(schemas_in_files) \
	intltool-extract.in \
	intltool-update.in \
	intltool-merge.in \
	COPYING-DOCS

CLEANFILES = \
	gchemutils.schemas \
	gchemutils.xml \
	$(mime_DATA) \
	$(schemas_DATA)

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
	intltool-merge.in \
	intltool-update.in
	ltmain.sh \
	missing \
	mkinstalldirs \
	Makefile.in

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper \
	--disable-schemas-install \
	--disable-update-databases \
	--with-mozilla-libdir=\$${libdir}/mozilla \
	--with-goffice-plugins-dir=\$${libdir}/goffice

mime_in_files = gchemutils.xml.in
mimedir = $(datadir)/mime/packages
mime_DATA = $(shell echo "$(mime_in_files)" | sed 's/.xml.in$$//;$(transform);s/$$/.xml/')

@INTLTOOL_XML_RULE@

schemas_in_files = gchemutils.schemas.in
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = $(shell echo "$(schemas_in_files)" | sed 's/.schemas.in$$//;$(transform);s/$$/.schemas/')

@INTLTOOL_SCHEMAS_RULE@

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif

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

install-data-local:
if GCONF_SCHEMAS_INSTALL
	for schema in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-install-rule $${schema} ; \
	done
endif #GCONF_SCHEMAS_INSTALL

uninstall-local:
if GCONF_SCHEMAS_INSTALL
	for schema in $(schemas_DATA) ; do \
		GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		    $(GCONFTOOL) --makefile-uninstall-rule $${schema} ; \
	done
endif #GCONF_SCHEMAS_INSTALL
