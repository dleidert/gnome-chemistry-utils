MAINTAINERCLEANFILES = Makefile.in

AM_CXXFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
AM_CFLAGS = -I$(top_srcdir) @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(datadir)/gchemutils\"

lib_LTLIBRARIES = libgcu.la

libgcu_la_SOURCES = \
	atom.cc \
	bond.cc \
	object.cc \
	document.cc \
	element.cc \
	isotope.cc \
	formula.cc \
	matrix.cc \
	matrix2d.cc \
	xml-utils.cc \
	application.cc \
	dialog.cc \
	chemistry.cc \
	gtkperiodic.c \
	gtkcomboperiodic.c \
	gtkcrystalviewer.cc \
	crystalview.cc \
	crystaldoc.cc \
	crystalatom.cc \
	crystalline.cc \
	crystalcleavage.cc \
	gtkchem3dviewer.cc

libgcu_la_headers = \
	atom.h \
	bond.h \
	object.h \
	document.h \
	element.h \
	isotope.h \
	formula.h \
	application.h \
	dialog.h \
	matrix.h \
	matrix2d.h \
	xml-utils.h \
	chemistry.h \
	gtkperiodic.h \
	gtkcomboperiodic.h \
	gtkcrystalviewer.h \
	crystalview.h \
	crystaldoc.h \
	crystalatom.h \
	crystalline.h \
	crystalcleavage.h \
	gtkchem3dviewer.h

libgcu_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@

gcuincludedir = $(includedir)/gchemutils/gcu
gcuinclude_HEADERS = $(libgcu_la_headers)
