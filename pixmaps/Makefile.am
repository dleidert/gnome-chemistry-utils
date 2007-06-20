MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(datadir)/gchemutils/pixmaps
pixmap_DATA = gcrystal_logo.png

icons = gcrystal32.png \
	gcrystal48.png \
	gcrystal72.png \
	gcrystal128.png \
	gchem3d-viewer32.png \
	gchem3d-viewer48.png \
	gchem3d-viewer72.png \
	gchem3d-viewer128.png \
	gchemcalc32.png \
	gchemcalc48.png \
	gchemcalc72.png \
	gchemcalc128.png \
	gchemtable32.png \
	gchemtable48.png \
	gchemtable72.png \
	gchemtable128.png

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
		$(INSTALL_DATA) $(srcdir)/gchem3d-viewer$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchem3d-viewer.png; \
		$(INSTALL_DATA) $(srcdir)/gchemcalc$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemcalc.png; \
		$(INSTALL_DATA) $(srcdir)/gchemtable$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemtable.png; \
	done

uninstall-hook:
	for i in 32 48 72 128; do \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchem3d-viewer.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemcalc.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemtable.png; \
	done
