libexec_PROGRAMS = babelserver

AM_CPPFLAGS = \
	-I$(top_srcdir) -I$(top_srcdir)/libs \
	$(openbabel_CFLAGS)

LIBS = \
	$(openbabel_LIBS)

babelserver_SOURCES = \
	babelserv.cc	\
	socket.cc	\
	socket.h
