## Makefile.am for gcheputils/tests

INCLUDES = -I$(top_srcdir)/gtkperiodic -I$(top_srcdir)/crystalviewer

DEPS = \
	$(top_builddir)/gtkperiodic/libgtkperiodic.la

LDADDS = $(top_builddir)/gtkperiodic/libgtkperiodic.la @GCU_LIBS@
AM_CFLAGS = @GCU_CFLAGS@

if WITH_GTKGL

noinst_PROGRAMS = 			\
	testgtkperiodic		\
	testgtkcrystalviewer

testgtkcrystalviewer_DEPENDENCIES = $(top_builddir)/crystalviewer/libgtkcrystalviewer.la $(top_builddir)/chemistry/libgcu.la

testgtkcrystalviewer_LDADD = $(top_builddir)/crystalviewer/libgtkcrystalviewer.la $(top_builddir)/chemistry/libgcu.la @GCU_LIBS@ -lstdc++

testgtkcrystalviewer_SOURCES =	\
	testgtkcrystalviewer.c 	

else

noinst_PROGRAMS = 			\
	testgtkperiodic

endif

testgtkperiodic_DEPENDENCIES = $(DEPS)

testgtkperiodic_LDADD = $(top_builddir)/gtkperiodic/libgtkperiodic.la $(top_builddir)/chemistry/libgcu.la $(LDADDS) -lstdc++

testgtkperiodic_SOURCES =	\
	testgtkperiodic.c 	

EXTRA_DIST = nickel.gcrystal
