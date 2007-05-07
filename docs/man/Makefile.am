EXTRA_DIST = \
	gcu_entities.dtd.in \
	$(gcu_man_src) \
	$(man1_MANS) \
	$(man3_MANS)

DISTCLEAN = gcu_entities.dtd

MAINTAINERCLEANFILES = Makefile.in

XMLLINT_FLAGS  += --path "$(VPATH)" --noout --valid 
XSLTPROC_FLAGS += --path "$(VPATH)" \
                  --param man.charmap.use.subset "0" \
                  --param make.year.ranges "1" \
                  --param make.single.year.ranges "1"

VPATH += @builddir@

gcu_man_src = \
	gchem3d-viewer.1.xml \
	gchemcalc.1.xml \
	gchemtable.1.xml \
	gcrystal.1.xml \
	libgcu.3.xml

man_MANS = gchem3d-viewer.1 gchemcalc.1 gchemtable.1 gcrystal.1 libgcu.3

SUFFIXES = .xml

.xml:
if HAVE_XSLTPROC
	$(XSLTPROC) $(XSLTPROC_FLAGS) $<
else
	echo "***** Cannot update the XML sources without `xsltproc'."
endif


check-local:
	@echo "***** Correct any errors before making a distribution."
if HAVE_XMLLINT
	$(XMLLINT) $(XMLLINT_FLAGS) $(gcu_man_src)
else
	@echo "***** Cannot check the XML sources without `xmllint'."
endif
if HAVE_MAN
	@for man in $(man_MANS) ; do \
		LANG=C MANWIDTH=80 $(MAN) $(srcdir)/$${man} > /dev/null ; \
	done
else
	@echo "***** Cannot check the manpages without `man'."
endif	

