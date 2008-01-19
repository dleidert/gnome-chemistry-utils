EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

AM_CFLAGS = -Werror-implicit-function-declaration
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/gcu

LDADD = $(top_builddir)/gcu/libgcu.la

DEFS += -DSRCDIR=\"@TESTSSRCDIR@\"

check_PROGRAMS = \
	testgtkperiodic \
	testgtkcrystalviewer \
	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES = testgtkcrystalviewer.c
testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c
testgtkperiodic_SOURCES = testgtkperiodic.c
