## Makefile.am for gcheputils/tests

INCLUDES = -I$(top_srcdir)/gtkperiodic -I$(top_srcdir)/crystalviewer

DEPS = \
	$(top_builddir)/gtkperiodic/libgtkperiodic.la

LDADDS = $(top_builddir)/gtkperiodic/libgtkperiodic.la @GCU_LIBS@
AM_CFLAGS = @GCU_CFLAGS@

if WITH_GTKGL

noinst_PROGRAMS = 			\
	testgtkperiodic		\
	testgtkcrystalviewer	\
	testgtkchem3dviewer \
	testbonobocontrol

testgtkcrystalviewer_DEPENDENCIES = $(top_builddir)/crystalviewer/libgtkcrystalviewer.la $(top_builddir)/chemistry/libgcu.la

testgtkcrystalviewer_LDADD = $(top_builddir)/crystalviewer/libgtkcrystalviewer.la $(top_builddir)/chemistry/libgcu.la @GCU_LIBS@ -lstdc++

testgtkcrystalviewer_SOURCES =	\
	testgtkcrystalviewer.c 	

testgtkchem3dviewer_DEPENDENCIES = $(top_builddir)/chem3d/libgtkchem3dviewer.la $(top_builddir)/chemistry/libgcu.la

testgtkchem3dviewer_LDADD = @GC3D_LIBS@ $(top_builddir)/chem3d/libgtkchem3dviewer.la $(top_builddir)/chemistry/libgcu.la @GCU_LIBS@ -lstdc++

AM_CFLAGS += @GC3D_CFLAGS@

testgtkchem3dviewer_SOURCES =	\
	testgtkchem3dviewer.c 	

testbonobocontrol_SOURCES = testbonobocontrol.c
testbonobocontrol_LDADD =  @GC3D_LIBS@

else

noinst_PROGRAMS = 			\
	testgtkperiodic

endif

testgtkperiodic_DEPENDENCIES = $(DEPS)

testgtkperiodic_LDADD = $(top_builddir)/gtkperiodic/libgtkperiodic.la $(top_builddir)/chemistry/libgcu.la $(LDADDS) -lstdc++

testgtkperiodic_SOURCES =	\
	testgtkperiodic.c 	

EXTRA_DIST = nickel.gcrystal methane.xyz
