
EXTRA_DIST = Doxyfile html-upstream.tar.gz

docsdir = $(datadir)/doc/gchemutils/reference
docs_DATA = html

html:
if DOXYGEN_AVAILABLE
	$(DOXYGEN) -u Doxyfile
	if [ -w Doxyfile.bak ]; then rm Doxyfile.bak; fi
	$(DOXYGEN) Doxyfile
else
	tar -xzf $(top_srcdir)/docs/reference/html-upstream.tar.gz
endif

install-docsDATA:
	$(mkinstalldirs) $(DESTDIR)$(docsdir)
	cp -r html $(DESTDIR)$(docsdir)/html

uninstall-docsDATA:
	rm -rf $(DESTDIR)$(docsdir)/html

distclean-generic:
	-rm -rf html
	-rm -f Doxyfile
