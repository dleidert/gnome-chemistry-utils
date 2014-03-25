MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
libgcu_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcu_@GCU_API_VER@_la_LIBADD = \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gdk_pixbuf_LIBS) \
		$(gio_LIBS) \
		$(cairo_LIBS) \
		-lGL
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
		cycle.cc \
		cylinder.cc	\
		dialog.cc \
		dialog-owner.cc \
		document.cc \
		element.cc \
		formula.cc \
		gldocument.cc	\
		glview.cc	\
		isotope.cc \
		loader.cc \
		loader-error.cc \
		matrix.cc \
		matrix2d.cc \
		molecule.cc \
		object.cc \
		residue.cc \
		spacegroup.cc   \
		sphere.cc	\
		transform3d.cc  \
		ui-builder.cc   \
		ui-manager.cc   \
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
		cycle.h \
		cylinder.h	\
		dialog.h \
		dialog-owner.h \
		document.h \
		element.h \
		formula.h \
		gldocument.h	\
		glview.h	\
		isotope.h \
		loader.h \
		loader-error.h \
		macros.h	\
		matrix.h	\
		matrix2d.h	\
		molecule.h \
		object.h \
		objprops.h \
		residue.h \
		spacegroup.h   \
		sphere.h	\
		structs.h   \
		transform3d.h  \
		ui-builder.h   \
		ui-manager.h   \
		value.h \
		vector.h \
		window.h \
		xml-utils.h
