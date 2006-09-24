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

man1_MANS = $(patsubst %.1.xml,%.1,$(filter %.1.xml,$(gcu_man_src)))
man3_MANS = $(patsubst %.3.xml,%.3,$(filter %.3.xml,$(gcu_man_src)))


% : %.xml
if HAVE_XSLTPROC
	$(XSLTPROC) $(XSLTPROC_FLAGS) $<
else
	$(warning "Cannot update the XML sources without `xsltproc'.")
endif


check-local:
	$(info "Correct any errors before making a distribution.")
if HAVE_XMLLINT
	$(XMLLINT) $(XMLLINT_FLAGS) $(gcu_man_src)
else
	$(warning "Cannot check the XML sources without `xmllint'.")
endif
if HAVE_MAN
	LANG=C MANWIDTH=80 $(MAN) $(addprefix $(srcdir)/,$(man1_MANS) $(man3_MANS)) > /dev/null
else
	$(warning "Cannot check the manpages without `man'.")
endif	

