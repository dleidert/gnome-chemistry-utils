MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(datadir)/gchemutils/@GCU_API_VER@/pixmaps
dist_pixmap_DATA = gcrystal_logo.png gchempaint_logo.png


iconsdir = $(datadir)/icons/hicolor/scalable/apps
dist_icons_DATA = \
	gchem3d.svg \
	gchemcalc.svg \
	gchempaint.svg \
	gchemtable.svg \
	gcrystal.svg \
	gspectrum.svg

install-data-hook:
	$(mkinstalldirs) $(datadir)/icons/hicolor/scalable/mimetypes; \
	$(INSTALL_DATA) $(srcdir)/gcrystal.svg \
		$(datadir)/icons/hicolor/scalable/mimetypes/application-x-gcrystal.svg; \
	$(INSTALL_DATA) $(srcdir)/gchempaint.svg \
		$(datadir)/icons/hicolor/scalable/mimetypes/application-x-gchempaint.svg;

uninstall-hook:
	rm $(datadir)/icons/hicolor/scalable/mimetypes/application-x-gcrystal.svg; \
	rm $(datadir)/icons/hicolor/scalable/mimetypes/application-x-gchempaint.svg;
