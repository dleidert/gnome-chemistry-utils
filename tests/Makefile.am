## Makefile.am for gcheputils/tests

INCLUDES = -I$(top_srcdir)/gcu

DEPS = \
	$(top_builddir)/gcu/libgcu.la

AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@ -Werror-implicit-function-declaration

noinst_PROGRAMS = 			\
	testgtkperiodic 			\
	testgtkcrystalviewer	\
	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES =	\
	testgtkcrystalviewer.c

testgtkcrystalviewer_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

testgtkchem3dviewer_SOURCES =	\
	testgtkchem3dviewer.c

testgtkchem3dviewer_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

testgtkperiodic_DEPENDENCIES = $(DEPS)
testgtkperiodic_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

testgtkperiodic_SOURCES =	\
	testgtkperiodic.c

EXTRA_DIST = nickel.gcrystal methane.xyz
