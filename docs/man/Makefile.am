EXTRA_DIST = \
	gcu_entities.dtd.in \
	gcu_man_legal.xml \
	$(gcu_man_src) \
	$(man_MANS)

DISTCLEAN = gcu_entities.dtd

MAINTAINERCLEANFILES = Makefile.in

XMLLINT_FLAGS  += --path "$(VPATH)" --noout --valid 
XSLTPROC_FLAGS += --path "$(VPATH)" \
                  --param man.charmap.use.subset "0" \
                  --param make.year.ranges "1" \
                  --param make.single.year.ranges "1"

VPATH += @builddir@

gcu_man_src = \
	gchem3d.1.xml \
	gchemcalc.1.xml \
	gchempaint.1.xml \
	gchemtable.1.xml \
	gcrystal.1.xml \
	gspectrum.1.xml

man_MANS = \
	gchem3d@STABILITY@.1 \
	gchemcalc@STABILITY@.1 \
	gchempaint@STABILITY@.1 \
	gchemtable@STABILITY@.1 \
	gcrystal@STABILITY@.1 \
	gspectrum@STABILITY@.1

SUFFIXES = .xml

%@STABILITY@.1 : %.1.xml
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

