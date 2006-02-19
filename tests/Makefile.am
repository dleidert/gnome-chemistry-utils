EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/gcu/libgcu.la
INCLUDES = -I$(top_srcdir)/gcu
LDADD = $(top_builddir)/gcu/libgcu.la @GCU_LIBS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@ -Werror-implicit-function-declaration

check_PROGRAMS = testgtkperiodic

testgtkperiodic_SOURCES = testgtkperiodic.c

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
testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c 	
testbonobocontrol_SOURCES = testbonobocontrol.c

endif #WITH_GTKGL

