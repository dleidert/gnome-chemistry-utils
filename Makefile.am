if WITH_GTKGL
SUBDIRS = po glade chemistry crystalviewer gtkperiodic chem3d tests dtds docs
else
SUBDIRS = po glade chemistry gtkperiodic tests dtds docs
endif

Mimedir = $(datadir)/mime-info
Mime_DATA = gchemutils.keys gchemutils.mime
keys_in_files = gchemutils.keys.in
@INTLTOOL_KEYS_RULE@

EXTRA_DIST=	\
	$(Mime_DATA) \
	gcu.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc
