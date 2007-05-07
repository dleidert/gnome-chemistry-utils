EXTRA_DIST = \
	gcu_entities.dtd.in \
	$(gcu_man_src) \
	$(man1_MANS) \
	$(man3_MANS)

DISTCLEAN = gcu_entities.dtd

MAINTAINERCLEANFILES = Makefile.in

XMLLINT_FLAGS  += --path "$(VPATH)" --noout --valid 
XSLTPROC_FLAGS += --path "$(VPATH)" --param man.charmap.use.subset "0"

VPATH += @builddir@

gcu_man_src = \
	gchem3d-viewer.1.xml \
	gchemcalc.1.xml \
	gchemtable.1.xml \
	libgcu.3.xml

man1_MANS = $(gcu_man_src:.1.xml=.1)
man3_MANS = $(gcu_man_src:.3.xml=.3)

SUFFIXES = .xml

.xml:
if HAVE_XSLTPROC
	$(XSLTPROC) $(XSLTPROC_FLAGS) $<
else
	echo "***** Cannot update the XML sources without `xsltproc'."
endif


check-local:
	echo "***** Correct any errors before making a distribution."
if HAVE_XMLLINT
	$(XMLLINT) $(XMLLINT_FLAGS) $(gcu_man_src)
else
	echo "***** Cannot check the XML sources without `xmllint'."
endif
if HAVE_MAN
	for man in $(gcu_man_src) ; do \
		LANG=C MANWIDTH=80 $(MAN) $(srcdir)/$${man} > /dev/null ; \
	done
else
	echo "***** Cannot check the manpages without `man'."
endif	

