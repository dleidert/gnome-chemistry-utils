MAINTAINERCLEANFILES = Makefile.in

man_MANS = \
	gchem3d.1 \
	gchemcalc.1 \
	gchempaint.1 \
	gchemtable.1 \
	gcrystal.1 \
	gspectrum.1

check-local:
	@echo "***** Correct any errors before making a distribution."
if HAVE_MAN
	@for man in $(man_MANS) ; do \
		LANG=C MANWIDTH=80 $(MAN) $(srcdir)/$${man} > /dev/null ; \
	done
else
	@echo "***** Cannot check the manpages without `man'."
endif	

