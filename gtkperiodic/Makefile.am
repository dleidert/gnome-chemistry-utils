lib_LTLIBRARIES= libgtkperiodic.la

libgtkperiodic_la_SOURCES = gtkperiodic.c
libgtkperiodic_la_headers = gtkperiodic.h
libgtkperiodic_la_LDFLAGS = -version-info  @GTKP_VERSION_INFO@ @GCU_LIBS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@  -Werror-implicit-function-declaration
gcuincludedir = $(includedir)/gcu/gtkperiodic
gcuinclude_HEADERS = $(libgtkperiodic_la_headers)
