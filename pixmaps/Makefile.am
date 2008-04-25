EXTRA_DIST = $(icons)

MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(datadir)/gchemutils/@GCU_API_VER@/pixmaps
dist_pixmap_DATA = gcrystal_logo.png gchempaint_logo.png

icons = \
	gcrystal32.png \
	gcrystal48.png \
	gcrystal72.png \
	gcrystal128.png \
	gchempaint32.png \
	gchempaint48.png \
	gchempaint72.png \
	gchempaint128.png \
	gchem3d32.png \
	gchem3d48.png \
	gchem3d72.png \
	gchem3d128.png \
	gchemcalc32.png \
	gchemcalc48.png \
	gchemcalc72.png \
	gchemcalc128.png \
	gchemtable32.png \
	gchemtable48.png \
	gchemtable72.png \
	gchemtable128.png \
	gspectrum32.png \
	gspectrum48.png \
	gspectrum72.png \
	gspectrum128.png

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
		$(INSTALL_DATA) $(srcdir)/gchem3d$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchem3d.png; \
		$(INSTALL_DATA) $(srcdir)/gchemcalc$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemcalc.png; \
		$(INSTALL_DATA) $(srcdir)/gchemtable$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemtable.png; \
		$(INSTALL_DATA) $(srcdir)/gspectrum$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gspectrum.png; \
	done

uninstall-hook:
	for i in 32 48 72 128; do \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gcrystal.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gchempaint.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchempaint.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchem3d.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemcalc.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gchemtable.png; \
		rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/gspectrum.png; \
	done
