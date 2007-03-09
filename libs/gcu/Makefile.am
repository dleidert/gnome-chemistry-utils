MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
AM_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade\"
lib_LTLIBRARIES= libgcu-unstable.la

libgcu_unstable_la_SOURCES = \
		atom.cc \
		bond.cc \
		object.cc \
		document.cc \
		element.cc \
		isotope.cc \
		formula.cc \
		value.cc \
		matrix.cc \
		matrix2d.cc \
		xml-utils.cc \
		application.cc \
		dialog.cc \
		dialog-owner.cc \
		filechooser.cc \
		chemistry.cc \
		gtkperiodic.c \
		gtkcomboperiodic.c \
		gtkcrystalviewer.cc \
		crystalview.cc \
		crystaldoc.cc \
		crystalatom.cc \
		crystalbond.cc \
		crystalline.cc \
		crystalcleavage.cc \
		chem3ddoc.cc	\
		gtkchem3dviewer.cc \
		gldocument.cc	\
		glview.cc

libgcu_unstable_la_headers = \
		atom.h \
		bond.h \
		object.h \
		document.h \
		element.h \
		isotope.h \
		formula.h \
		value.h \
		application.h \
		dialog.h \
		dialog-owner.h \
		filechooser.h \
		macros.h	\
		matrix.h	\
		matrix2d.h	\
		xml-utils.h \
		chemistry.h \
		gtkperiodic.h \
		gtkcomboperiodic.h \
		gtkcrystalviewer.h \
		crystalview.h \
		crystaldoc.h \
		crystalatom.h \
		crystalbond.h \
		crystalline.h \
		crystalcleavage.h \
		chem3ddoc.h	\
		gtkchem3dviewer.h \
		gldocument.h	\
		glview.h

gcuincludedir = $(includedir)/gcu-unstable/gcu
gcuinclude_HEADERS = $(libgcu_unstable_la_headers)
