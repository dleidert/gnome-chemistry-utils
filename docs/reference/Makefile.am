EXTRA_DIST = html-upstream.tar.gz

docsdir = $(datadir)/doc/gchemutils/reference
docs_DATA = html

html:
if DOXYGEN_AVAILABLE
	$(DOXYGEN) -u Doxyfile
	if [ -w Doxyfile.bak ]; then rm Doxyfile.bak; fi
	$(DOXYGEN) Doxyfile
else
	if [ -r $(top_srcdir)/docs/reference/html-upstream.tar.gz ];
	then
		tar -xzf $(top_srcdir)/docs/reference/html-upstream.tar.gz;
	fi
endif

install-docsDATA:
	$(mkinstalldirs) $(DESTDIR)$(docsdir)
	cp -r html $(DESTDIR)$(docsdir)/html

uninstall-docsDATA:
	rm -rf $(DESTDIR)$(docsdir)/html

distclean-generic:
	-rm -rf html
	-rm -f Doxyfile

html-upstream.tar.gz: html
	if [ ! -d html ]; then echo "You need doxygen to make dist"; fi
	tar czf html-upstream.tar.gz html

# disable the check to have make distcheck work (html-upstream.tar.gz).
distuninstallcheck:
	@:
