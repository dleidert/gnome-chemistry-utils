EXTRA_DIST = $(icons)

MAINTAINERCLEANFILES = Makefile.in

pixmapdir = $(pkgdatadir)/pixmaps

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
		$(mkinstalldirs) $(DESTDIR)$(datadir)/icons/hicolor/"$$i"x"$$i"/mimetypes; \
		$(mkinstalldirs) $(DESTDIR)$(datadir)/icons/hicolor/"$$i"x"$$i"/apps; \
		list='gchem3d gchemcalc gchempaint gchemtable gcrystal gspectrum'; for p in $$list; do \
			f=`echo "$$p" | sed '$(transform)'`; \
			$(INSTALL_DATA) $(srcdir)/$$p$$i.png \
			    $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/$$f.png; \
		done ; \
		$(INSTALL_DATA) $(srcdir)/gcrystal$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		$(INSTALL_DATA) $(srcdir)/gchempaint$$i.png \
			$(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gchempaint.png; \
	done

uninstall-hook:
	for i in 32 48 72 128; do \
		list='gchem3d gchemcalc gchempaint gchemtable gcrystal gspectrum'; for p in $$list; do \
			f=`echo "$$p" | sed '$(transform)'`; \
			rm $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/apps/$$f.png; \
		done ; \
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gcrystal.png; \
		rm -f $(DESTDIR)$(datadir)/icons/hicolor/$$i"x"$$i/mimetypes/gnome-mime-application-x-gchempaint.png; \
	done
