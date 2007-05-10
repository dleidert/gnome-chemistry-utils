MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(datadir)/gchemutils-unstable/pixmaps
pixmap_DATA = gcrystal_logo.png gchempaint_logo.png

icons = gcrystal32.png \
	gcrystal48.png \
	gcrystal72.png \
	gcrystal128.png \
	gchempaint32.png \
	gchempaint48.png \
	gchempaint72.png \
	gchempaint128.png

EXTRA_DIST = \
	$(pixmap_DATA) \
	$(icons)

install-data-hook:
	for i in 32 48 72 128; do \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes; \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps; \
		$(INSTALL_DATA) $(srcdir)/gcrystal$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		$(INSTALL_DATA) $(srcdir)/gcrystal$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gcrystal.png; \
		$(INSTALL_DATA) $(srcdir)/gchempaint$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gchempaint.png; \
		$(INSTALL_DATA) $(srcdir)/gchempaint$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchempaint.png; \
	done

uninstall-hook:
	for i in 32 48 72 128; do \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gcrystal.png; \
	done
