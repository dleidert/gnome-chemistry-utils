MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(datadir)/@PACKAGE@/pixmaps
pixmap_DATA = gcrystal_logo.png

icons = gcrystal32.png \
	gcrystal48.png \
	gcrystal72.png \
	gcrystal128.png

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
	done

uninstall-hook:
	for i in 32 48 72 128; do \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gcrystal.png; \
	done
