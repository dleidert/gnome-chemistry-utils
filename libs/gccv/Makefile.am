MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgccv-@GCU_API_VER@.la

noinst_HEADERS = \
	arrow.h		\
	bezier-arrow.h  \
	canvas.h	\
	circle.h	\
	client.h	\
	fill-item.h	\
	group.h		\
	hash.h		\
	item.h		\
	item-client.h	\
	line.h	\
	line-item.h	\
	poly-line.h	\
	rectangle.h	\
	squiggle.h \
	structs.h \
	text.h \
	text-tag.h  \
	wedge.h

libgccv_@GCU_API_VER@_la_SOURCES =	\
	arrow.cc	\
	bezier-arrow.cc  \
	canvas.cc	\
	circle.cc	\
	client.cc	\
	fill-item.cc	\
	group.cc	\
	hash.cc		\
	item.cc		\
	item-client.cc	\
	line.cc	\
	line-item.cc	\
	poly-line.cc	\
	rectangle.cc	\
	squiggle.cc \
	text.cc \
	text-tag.cc	\
	wedge.cc

libgccv_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgccv_@GCU_API_VER@_la_LIBADD = \
		@cairo_LIBS@ \
		@gtk_LIBS@ \
		@goffice_LIBS@
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@goffice_CFLAGS@ \
		@GCU_CFLAGS@
