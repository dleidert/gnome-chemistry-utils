EXTRA_DIST = nickel.gcrystal methane.xyz

MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/gcu $(gtk_CFLAGS) $(gsf_CFLAGS)
DEPS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
LIBS = $(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
	$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
	$(gtk_LIBS) $(xml_LIBS)
AM_CFLAGS = -I$(top_srcdir)/libs $(GCU_CFLAGS) -Werror-implicit-function-declaration
DEFS += -DSRCDIR=\"$(TESTSSRCDIR)\"

testgcrcrystalviewer_LDADD = $(top_builddir)/libs/gcr/libgcrystal-@GCU_API_VER@.la
testbabelserver_CFLAGS = -DLIBEXECDIR=\"$(libexecdir)\"

check_PROGRAMS = \
	testgcuperiodic \
	testgcrcrystalviewer \
	testgcuchem3dviewer \
	testbabelserver

testgcrcrystalviewer_SOURCES = testgcrcrystalviewer.c
testgcuchem3dviewer_SOURCES = testgcuchem3dviewer.c
testgcuperiodic_SOURCES = testgcuperiodic.c
testbabelserver_SOURCES = testbabelserver.c
