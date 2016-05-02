MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgccv-@GCU_API_VER@.la

noinst_HEADERS = \
	arc.h  \
	arrow.h		\
	bezier-arrow.h  \
	brackets.h	\
	canvas.h	\
	circle.h	\
	client.h	\
	equation.h	\
	fill-item.h	\
	group.h		\
	hash.h		\
	item.h		\
	item-client.h	\
	leaf.h	\
	line.h	\
	line-item.h	\
	path.h	 \
	polygon.h	\
	poly-line.h	\
	rectangle.h	\
	squiggle.h \
	structs.h \
	text.h \
	text-client.h	\
	text-tag.h  \
	wedge.h

libgccv_@GCU_API_VER@_la_SOURCES =	\
	arc.cc  \
	arrow.cc	\
	bezier-arrow.cc  \
	brackets.cc	\
	canvas.cc	\
	circle.cc	\
	client.cc	\
	equation.cc	\
	fill-item.cc	\
	group.cc	\
	hash.cc		\
	item.cc		\
	item-client.cc	\
	leaf.cc	\
	line.cc	\
	line-item.cc	\
	path.cc	 \
	polygon.cc	\
	poly-line.cc	\
	rectangle.cc	\
	squiggle.cc \
	text.cc \
	text-client.cc	\
	text-tag.cc	\
	wedge.cc

libgccv_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgccv_@GCU_API_VER@_la_LIBADD = \
		$(cairo_LIBS) \
		$(gtk_LIBS) \
		$(goffice_LIBS)
AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
