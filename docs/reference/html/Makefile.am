HTMLDIR = $(datadir)/doc/gchemutils/reference/html

install-data-local:
	$(mkinstalldirs) $(HTMLDIR); \
  for i in *.html *.png *.css; do \
	echo '-- Installing '$$i ; \
	$(INSTALL_DATA) $$i $(HTMLDIR); \
  done

dist-hook: 
	-cp $(srcdir)/*.html $(distdir)
	-cp $(srcdir)/*.png $(distdir)
	-cp $(srcdir)/*.css $(distdir)
