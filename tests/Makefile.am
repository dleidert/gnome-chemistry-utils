EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/gcu

DEPS = $(top_builddir)/gcu/libgcu.la

AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@ -Werror-implicit-function-declaration

check_PROGRAMS = testgtkperiodic

#TESTS = testgtkperiodic

if WITH_GTKGL

check_PROGRAMS += \
	testgtkcrystalviewer \
	testgtkchem3dviewer \
	testbonobocontrol

#TESTS += \
#	testgtkcrystalviewer \
#	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES = testgtkcrystalviewer.c 	
testgtkcrystalviewer_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c 	
testgtkchem3dviewer_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

testbonobocontrol_SOURCES = testbonobocontrol.c
testbonobocontrol_LDADD = @GCU_LIBS@

endif #WITH_GTKGL

testgtkperiodic_DEPENDENCIES = $(DEPS)
testgtkperiodic_SOURCES = testgtkperiodic.c
testgtkperiodic_LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@

