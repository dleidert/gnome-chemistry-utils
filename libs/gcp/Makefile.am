MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES= libgchempaint-@GCU_API_VER@.la

libgchempaint_@GCU_API_VER@_la_SOURCES = \
		application.cc \
		plugin.cc \
		settings.cc \
		document.cc \
		target.cc \
		window.cc \
		operation.cc \
		arrow.cc \
		atom.cc \
		electron.cc \
		bond.cc \
		residue.cc \
		fragment-atom.cc \
		fragment-residue.cc \
		fragment.cc \
		molecule.cc \
		reaction.cc \
		reactant.cc \
		reaction-step.cc \
		reaction-arrow.cc \
		reaction-operator.cc \
		reaction-prop.cc \
		reaction-prop-dlg.cc \
		mesomery.cc \
		mesomer.cc \
		mesomery-arrow.cc \
		text.cc \
		text-object.cc \
		docprop.cc \
		fontsel.cc \
		newfiledlg.cc \
		preferences.cc \
		stringdlg.cc \
		about.cc \
		tool.cc \
		tools.cc \
		view.cc \
		widgetdata.cc \
		zoomdlg.cc \
		theme.cc \
		Hposdlg.cc

noinst_HEADERS = \
		application.h \
		plugin.h \
		settings.h \
		document.h \
		target.h \
		window.h \
		operation.h \
		arrow.h \
		atom.h \
		electron.h \
		bond.h \
		residue.h \
		fragment-atom.h \
		fragment-residue.h \
		fragment.h \
		molecule.h \
		reaction.h \
		reactant.h \
		reaction-step.h \
		reaction-arrow.h \
		reaction-operator.h \
		reaction-prop.h \
		reaction-prop-dlg.h \
		mesomery.h \
		mesomer.h \
		mesomery-arrow.h \
		text.h \
		text-object.h \
		docprop.h \
		fontsel.h \
		newfiledlg.h \
		preferences.h \
		stringdlg.h \
		about.h \
		tool.h \
		tools.h \
		view.h \
		widgetdata.h \
		zoomdlg.h \
		theme.h \
		Hposdlg.h
		
libgchempaint_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgchempaint_@GCU_API_VER@_la_LIBADD = \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/canvas/libgcpcanvas-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		@gnome_canvas_CFLAGS@ \
		@goffice_CFLAGS@ \
		@openbabel_CFLAGS@ \
		@GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
		-DGLADEDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/glade/paint\" \
		-DPLUGINSDIR=\"$(libdir)/gchemutils/@GCU_API_VER@/plugins/paint\" \
		-DPIXMAPSDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/pixmaps\" \
		-DLOCALEDIR=\"$(localedir)\"

