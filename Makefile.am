if WITH_GTKGL
SUBDIRS = po glade chemistry crystalviewer gtkperiodic chem3d tests dtds
else
SUBDIRS = po glade chemistry gtkperiodic tests dtds
endif

EXTRA_DIST=	\
	gcu.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc
