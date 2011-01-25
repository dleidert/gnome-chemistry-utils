MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(gtkglext_CFLAGS) \
		$(GCU_CFLAGS)
libgcu_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcu_@GCU_API_VER@_la_LIBADD = \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gtkglext_LIBS)
DEFS += -DDATADIR=\"$(datadir)\" \
		-DLOCALEDIR=\"$(localedir)\" \
		-DLIBEXECDIR=\"$(libexecdir)\" \
		-DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
		-DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/libgcu\" \
		-DGCU_PLUGINS_DIR=\"$(libdir)/gchemutils/@GCU_API_VER@/plugins\"
lib_LTLIBRARIES= libgcu-@GCU_API_VER@.la

libgcu_@GCU_API_VER@_la_SOURCES = \
		application.cc \
		atom.cc \
		bond.cc \
		chain.cc \
		chem3ddoc.cc	\
		chemistry.cc \
		cmd-context.cc \
		cmd-context-gtk.cc \
		cycle.cc \
		cylinder.cc	\
		dialog.cc \
		dialog-owner.cc \
		document.cc \
		element.cc \
		filechooser.cc \
		formula.cc \
		gcuchem3dviewer.cc \
		gcucomboperiodic.c \
		gcuperiodic.c \
		gcuspectrumviewer.cc \
		gldocument.cc	\
		glview.cc	\
		isotope.cc \
		loader.cc \
		loader-error.cc \
		matrix.cc \
		matrix2d.cc \
		message.cc \
		molecule.cc \
		object.cc \
		printable.cc \
		print-setup-dlg.cc \
		residue.cc \
		spectrumdoc.cc \
		spectrumview.cc \
		spacegroup.cc   \
		sphere.cc	\
		transform3d.cc  \
		ui-builder.cc   \
		value.cc \
		vector.cc \
		window.cc \
		xml-utils.cc

noinst_HEADERS = \
		application.h \
		atom.h \
		bond.h \
		chain.h \
		chem3ddoc.h	\
		chemistry.h \
		cmd-context.h \
		cmd-context-gtk.h \
		cycle.h \
		cylinder.h	\
		dialog.h \
		dialog-owner.h \
		document.h \
		element.h \
		filechooser.h \
		formula.h \
		gcuchem3dviewer.h \
		gcucomboperiodic.h \
		gcuperiodic.h \
		gcuspectrumviewer.h \
		gldocument.h	\
		glview.h	\
		isotope.h \
		loader.h \
		loader-error.h \
		macros.h	\
		matrix.h	\
		matrix2d.h	\
		message.h \
		molecule.h \
		object.h \
		objprops.h \
		printable.h \
		print-setup-dlg.h \
		residue.h \
		spectrumdoc.h \
		spectrumview.h \
		spacegroup.h   \
		sphere.h	\
		structs.h   \
		transform3d.h  \
		ui-builder.h   \
		value.h \
		vector.h \
		window.h \
		xml-utils.h
