SUBDIRS = po glade chemistry
if WITH_GTKGL
SUBDIRS += crystalviewer chem3d
endif
SUBDIRS += gtkperiodic tests dtds docs

Mimedir = $(datadir)/mime-info
Mime_DATA = gchemutils.keys gchemutils.mime
keys_in_files = gchemutils.keys.in
@INTLTOOL_KEYS_RULE@

mimedir = $(datadir)/mime/packages
mime_in_files = gchemutils.xml.in
mime_DATA = $(mime_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST=	\
	$(keys_in_files) \
	gchemutils.mime \
	$(mime_in_files) \
	gcu.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc

install-data-hook:
	update-mime-database $(datadir)/mime

uninstall-hook:
	update-mime-database $(datadir)/mime
