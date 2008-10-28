EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
AM_CPPFLAGS = -I$(top_srcdir)/gcu
AM_CFLAGS = -I$(top_srcdir)/libs @GCU_CFLAGS@ -Werror-implicit-function-declaration
LDADD = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la @GCU_LIBS@
DEFS += -DSRCDIR=\"$(abs_srcdir)\"

check_PROGRAMS = \
	testgtkperiodic \
	testgtkcrystalviewer \
	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES = testgtkcrystalviewer.c
testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c
testgtkperiodic_SOURCES = testgtkperiodic.c
