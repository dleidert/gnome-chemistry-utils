SUBDIRS = po glade gcu tests dtds docs

if GNOME24_SUPPORT
Mimedir = $(datadir)/mime-info
Mime_DATA = gchemutils.keys gchemutils.mime
keys_in_files = gchemutils.keys.in
@INTLTOOL_KEYS_RULE@
endif

mimedir = $(datadir)/mime/packages
mime_in_files = gchemutils.xml.in
mime_DATA = $(mime_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST=	\
	gchempaint.keys.in \
	gchempaint.mime \
	$(mime_in_files) \
	gcu.pc.in \
	intltool-extract.in	\
	intltool-update.in	\
	intltool-merge.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gcu.pc

CLEANFILES = \
	gchemutils.keys \
	gchemutils.xml
	
DISTCLEANFILES = \
	intltool-extract \
	intltool-update \
	intltool-merge

install-data-hook:
if UPDATE_DATABASES
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif
endif

uninstall-hook:
if UPDATE_DATABASES
# bad hacking for make distcheck !
	if test -w $(datadir)/mime/chemical/x-xyz.xml; then \
		rm $(datadir)/mime/chemical/x-xyz.xml; \
	fi
	if test -w $(datadir)/mime/chemical/x-mdl-molfile.xml; then \
		rm $(datadir)/mime/chemical/x-mdl-molfile.xml; \
	fi
	if test -w $(datadir)/mime/chemical/x-pdb.xml; then \
		rm $(datadir)/mime/chemical/x-pdb.xml; \
	fi
	update-mime-database $(datadir)/mime
if WITH_UPDATE_DESKTOP
	update-desktop-database $(datadir)/applications
endif
endif

# remove the following when we don't install mime type anymore
distuninstallcheck:
             @:

distcleancheck_listfiles = \
       find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'
