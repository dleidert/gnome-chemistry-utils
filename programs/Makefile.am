bin_PROGRAMS = gchemcalc

gchemcalc_SOURCES = gchemcalc.cc

DEPS = \
	$(top_builddir)/gcu/libgcu-0.5.la

INCLUDES = @GCU_CFLAGS@
gchemcalc_LDADD = $(top_builddir)/gcu/libgcu-0.5.la @GCU_LIBS@
