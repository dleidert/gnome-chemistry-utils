EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

if GCU_NEEDS_GCONF
INCLUDES = -I$(top_srcdir)/gcu $(gtk_CFLAGS) $(gsf_CFLAGS) $(gnome_vfs_CFLAGS)
else
INCLUDES = -I$(top_srcdir)/gcu $(gtk_CFLAGS) $(gsf_CFLAGS)
endif
DEPS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(gtk_LIBS) $(xml_LIBS)
AM_CFLAGS = -I$(top_srcdir)/libs $(GCU_CFLAGS) -Werror-implicit-function-declaration
DEFS += -DSRCDIR=\"$(TESTSSRCDIR)\"

check_PROGRAMS = \
	testgtkperiodic \
	testgtkcrystalviewer \
	testgtkchem3dviewer

testgtkcrystalviewer_SOURCES = testgtkcrystalviewer.c
testgtkchem3dviewer_SOURCES = testgtkchem3dviewer.c
testgtkperiodic_SOURCES = testgtkperiodic.c
