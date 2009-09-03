EXTRA_DIST = html-upstream.tar.gz

CLEANFILES = html_stamp Doxyfile.bak

DISTCLEANFILES = Doxyfile

MAINTAINERCLEANFILES = Makefile.in

# better use the docdir/htmldir variable, configurable via --docdir|--htmldir?
# htmldir = $(datadir)/doc/gchemutils/@GCU_API_VER@/reference
# html_DATA = html/*.*

all: html/*.*

html/*.*: html_stamp

if DOXYGEN_AVAILABLE

html_stamp:
	$(DOXYGEN) -u Doxyfile
	$(DOXYGEN) Doxyfile
	touch $@

html-upstream.tar.gz: html/*.*
	tar -czf $@ html/*.*

else #DOXYGEN_AVAILABLE

## Make sure, that html-upstream.tar.gz exists. If not, fail.
## This will force the existance of documentation!
html_stamp: html-upstream.tar.gz
	tar -xzf $<
	touch html_stamp

## Fail during make dist, because the documentation cannot be created.
html-upstream.tar.gz: ; echo "Error: You need doxygen to create the documentation." && exit 1

endif #DOXYGEN_AVAILABLE

clean-local:
	rm -rf html/*.*

## Make distcheck happy for maintainers as well
dist-hook:
	rm -f html-upstream.tar.gz
