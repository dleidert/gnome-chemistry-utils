MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgcpcanvas-@GCU_API_VER@.la

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

libgcpcanvas_@GCU_API_VER@_la_SOURCES =	\
	gprintable.c \
	pango-layout-to-svg.c \
	gcp-canvas-group.c \
	gcp-canvas-line.c \
	gcp-canvas-pango.c \
	gcp-canvas-shape.c \
	gcp-canvas-bpath.c \
	gcp-canvas-rect-ellipse.c \
	gcp-canvas-polygon.c

libgcpcanvas_@GCU_API_VER@_la_LDFLAGS = \
	-version-info  @GCU_VERSION_INFO@
libgcpcanvas_@GCU_API_VER@_la_LIBADD = \
	@gnome_canvas_LIBS@ \
	@xml_LIBS@
INCLUDES = \
	@gnome_canvas_CFLAGS@ \
	@xml_CFLAGS@
	@GCU_CFLAGS@
