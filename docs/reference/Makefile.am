EXTRA_DIST = html-upstream.tar.gz

CLEANFILES = html_stamp Doxyfile.bak

DISTCLEANFILES = Doxyfile

MAINTAINERCLEANFILES = Makefile.in

docsdir = $(datadir)/doc/gchemutils/reference
docs_DATA = html/*

html/*: html_stamp

html_stamp:
if DOXYGEN_AVAILABLE
	$(DOXYGEN) -u Doxyfile
	$(DOXYGEN) Doxyfile
else
	if [ -r $(top_srcdir)/docs/reference/html-upstream.tar.gz ]; then \
		tar -xzf $(top_srcdir)/docs/reference/html-upstream.tar.gz; \
	fi
endif #DOXYGEN_AVAILABLE
	touch html_stamp

clean-local:
	rm -rf html

html-upstream.tar.gz: html/*
	if [ ! -d html ]; then echo "You need doxygen to make dist"; fi
	tar czf html-upstream.tar.gz html

dist-hook:
	rm -f html-upstream.tar.gz

