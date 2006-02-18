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
	tar -xzf $(top_srcdir)/docs/reference/html-upstream.tar.gz
endif #DOXYGEN_AVAILABLE
	touch html_stamp

clean-local:
	rm -rf html

if DOXYGEN_AVAILABLE
html-upstream.tar.gz: html/*
	tar czf html-upstream.tar.gz html
else
html-upstream.tar.gz: ; $(error "Error: You need doxygen to 'make dist'.")
endif #DOXYGEN_AVAILABLE

dist-hook:
	rm -f html-upstream.tar.gz

