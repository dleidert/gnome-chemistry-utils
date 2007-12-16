MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
AM_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
DEFS += -DDATADIR=\"$(datadir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade\" \
	-DGCU_PLUGINS_DIR=\"$(libdir)/gchemutils/@VERSION@\"
lib_LTLIBRARIES= libgcu-unstable.la

libgcu_unstable_la_SOURCES = \
		application.cc \
		atom.cc \
		bond.cc \
		chem3ddoc.cc	\
		chemistry.cc \
		cmd-context.cc \
		crystalatom.cc \
		crystalbond.cc \
		crystalcleavage.cc \
		crystaldoc.cc \
		crystalline.cc \
		crystalview.cc \
		cylinder.cc	\
		dialog.cc \
		dialog-owner.cc \
		document.cc \
		element.cc \
		filechooser.cc \
		formula.cc \
		gldocument.cc	\
		glview.cc	\
		gtkchem3dviewer.cc \
		gtkcomboperiodic.c \
		gtkcrystalviewer.cc \
		gtkperiodic.c \
		gtkspectrumviewer.cc \
		isotope.cc \
		loader.cc \
		matrix.cc \
		matrix2d.cc \
		object.cc \
		residue.cc \
		spectrumdoc.cc \
		spectrumview.cc \
		sphere.cc	\
		value.cc \
		vector.cc \
		xml-utils.cc

noinst_HEADERS = \
		application.h \
		atom.h \
		bond.h \
		chem3ddoc.h	\
		chemistry.h \
		cmd-context.h \
		crystalatom.h \
		crystalbond.h \
		crystalcleavage.h \
		crystaldoc.h \
		crystalline.h \
		crystalview.h \
		cylinder.h	\
		dialog.h \
		dialog-owner.h \
		document.h \
		element.h \
		filechooser.h \
		formula.h \
		gldocument.h	\
		glview.h	\
		gtkchem3dviewer.h \
		gtkcomboperiodic.h \
		gtkcrystalviewer.h \
		gtkperiodic.h \
		gtkspectrumviewer.h \
		isotope.h \
		loader.h \
		macros.h	\
		matrix.h	\
		matrix2d.h	\
		object.h \
		residue.h \
		spectrumdoc.h \
		spectrumview.h \
		sphere.h	\
		value.h \
		vector.h \
		xml-utils.h
