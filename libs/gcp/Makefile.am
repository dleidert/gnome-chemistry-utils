MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES= libgchempaint-unstable.la

libgchempaint_unstable_la_SOURCES = \
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
		fragment-residue.cc \
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
		
libgchempaint_unstable_la_LDFLAGS = -release $(VERSION) \
	$(top_builddir)/libs/gcu/libgcu-unstable.la \
	$(top_builddir)/libs/canvas/libgcpcanvas-unstable.la \
	@GCU_LIBS@
AM_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libs @GCU_CFLAGS@
DEFS += -DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils-unstable\" \
	-DGLADEDIR=\"$(datadir)/gchemutils-unstable/glade/paint\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils-unstable/plugins/paint\" \
	-DPIXMAPSDIR=\"$(datadir)/gchemutils-unstable/pixmaps\" \
	-DLOCALEDIR=\"$(localedir)\"

