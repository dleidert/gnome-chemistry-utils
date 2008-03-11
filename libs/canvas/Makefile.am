MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgcpcanvas-unstable.la

noinst_HEADERS = \
	gprintable.h \
	gnome-canvas-shape-private.h \
	gcp-canvas-group.h \
	gcp-canvas-line.h \
	gcp-canvas-pango.h \
	gcp-canvas-shape.h \
	gcp-canvas-bpath.h \
	gcp-canvas-rect-ellipse.h \
	gcp-canvas-polygon.h \
	gcp-canvas-i18n.h

libgcpcanvas_unstable_la_SOURCES =	\
	gprintable.c \
	pango-layout-to-svg.c \
	gcp-canvas-group.c \
	gcp-canvas-line.c \
	gcp-canvas-pango.c \
	gcp-canvas-shape.c \
	gcp-canvas-bpath.c \
	gcp-canvas-rect-ellipse.c \
	gcp-canvas-polygon.c

libgcpcanvas_unstable_la_LDFLAGS = -release $(VERSION) @GCU_LIBS@
AM_CPPFLAGS = @GCU_CFLAGS@
