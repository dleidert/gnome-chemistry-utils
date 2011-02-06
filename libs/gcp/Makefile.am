BUILT_SOURCES = gcp-stock-pixbufs.h

CLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = \
	$(IMAGES)
	
IMAGES = \
	unallowed.png

VARIABLES = \
	gcp_unallowed			$(srcdir)/unallowed.png

$(BUILT_SOURCES): $(IMAGES)
	$(GDK_PIXBUF_CSOURCE) --raw --build-list $(VARIABLES) > $@

lib_LTLIBRARIES= libgcp-@GCU_API_VER@.la

libgcp_@GCU_API_VER@_la_SOURCES = \
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
		brackets.cc \
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
		mechanism-arrow.cc \
		mechanism-step.cc \
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
		electron.h \
		arrow.h \
		atom.h \
		bond.h \
		brackets.h \
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
		mechanism-arrow.h \
		mechanism-step.h \
		text.h \
		text-editor.h \
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
		
libgcp_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcp_@GCU_API_VER@_la_LIBADD = \
		$(gtk_LIBS) $(gsf_LIBS) $(goffice_LIBS) -ldl \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la
INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
DEFS += -DGCU_API_VER=\"@GCU_API_VER@\" \
	-DDATADIR=\"$(datadir)\" -DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
	-DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/paint\" \
	-DPLUGINSDIR=\"$(libdir)/gchemutils/@GCU_API_VER@/plugins/paint\" \
	-DPIXMAPSDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/pixmaps\" \
	-DLOCALEDIR=\"$(localedir)\"

