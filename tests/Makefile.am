## Makefile.am for gcheputils/tests

INCLUDES = -I$(top_srcdir)/gtkperiodic -I$(top_srcdir)/crystalviewer

DEPS = \
	$(top_builddir)/gtkperiodic/libgtkperiodic-0.3.la

AM_CFLAGS = @GCU_CFLAGS@ -Werror-implicit-function-declaration

noinst_PROGRAMS = 			\
	testgtkperiodic

if WITH_GTKGL

noinst_PROGRAMS += 			\
	testgtkcrystalviewer	\
	testgtkchem3dviewer \
	testbonobocontrol

testgtkcrystalviewer_SOURCES =	\
	testgtkcrystalviewer.c 	

testgtkcrystalviewer_LDADD = $(top_builddir)/gcu/libgcu-0.3.la @GCU_LIBS@

testgtkchem3dviewer_SOURCES =	\
	testgtkchem3dviewer.c 	

testgtkchem3dviewer_LDADD = $(top_builddir)/gcu/libgcu-0.3.la @GCU_LIBS@

testbonobocontrol_SOURCES = testbonobocontrol.c
testbonobocontrol_LDADD =  @GCU_LIBS@

endif

testgtkperiodic_DEPENDENCIES = $(DEPS)
testgtkperiodic_LDADD = $(top_builddir)/gcu/libgcu-0.3.la @GCU_LIBS@

testgtkperiodic_SOURCES =	\
	testgtkperiodic.c 	

EXTRA_DIST = nickel.gcrystal methane.xyz
