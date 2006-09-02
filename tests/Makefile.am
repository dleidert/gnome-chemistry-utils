EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/gcu
DEPS = $(top_builddir)/gcu/libgcu-unstable.la
LDADD = $(top_builddir)/gcu/libgcu-unstable.la @GCU_LIBS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@ -Werror-implicit-function-declaration
DEFS += -DSRCDIR=\"@TESTSSRCDIR@\"

check_PROGRAMS = \
	testgtkperiodic \
	testgtkcrystalviewer \
	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES = testgtkcrystalviewer.c
testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c
testgtkperiodic_SOURCES = testgtkperiodic.c
