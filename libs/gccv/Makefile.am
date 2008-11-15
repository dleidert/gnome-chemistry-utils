MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES = libgccv-@GCU_API_VER@.la

noinst_HEADERS = \
	canvas.h	\
	client.h	\
	fill-item.h	\
	group.h		\
	hash.h		\
	item.h		\
	item-client.h	\
	line.h	\
	line-item.h	\
	rectangle.h	\
	squiggle.h \
	structs.h \
	text.h \
	wedge.h

libgccv_@GCU_API_VER@_la_SOURCES =	\
	canvas.cc	\
	client.cc	\
	fill-item.cc	\
	group.cc	\
	hash.cc		\
	item.cc		\
	item-client.cc	\
	line.cc	\
	line-item.cc	\
	rectangle.cc	\
	squiggle.cc \
	text.cc \
	wedge.cc

libgccv_@GCU_API_VER@_la_LDFLAGS = - -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
