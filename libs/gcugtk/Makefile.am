MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(gtkglext_CFLAGS) \
		$(GCU_CFLAGS)
libgcugtk_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcugtk_@GCU_API_VER@_la_LIBADD = \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(gtkglext_LIBS) \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la
DEFS += -DDATADIR=\"$(datadir)\" \
		-DLOCALEDIR=\"$(localedir)\" \
		-DLIBEXECDIR=\"$(libexecdir)\" \
		-DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
		-DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/libgcu\" \
		-DGCU_PLUGINS_DIR=\"$(libdir)/gchemutils/@GCU_API_VER@/plugins\"
lib_LTLIBRARIES= libgcugtk-@GCU_API_VER@.la

libgcugtk_@GCU_API_VER@_la_SOURCES = \
		application.cc \
		cmd-context-gtk.cc \
		gcuchem3dviewer.cc \
		gcucomboperiodic.c \
		gcuperiodic.c \
		gcuspectrumviewer.cc \
		glview.cc	\
		printable.cc \
		print-setup-dlg.cc	\
		spectrumdoc.cc \
		spectrumview.cc \
		ui-manager.cc

noinst_HEADERS = \
		application.h \
		cmd-context-gtk.h \
		gcuchem3dviewer.h \
		gcucomboperiodic.h \
		gcuperiodic.h \
		gcuspectrumviewer.h \
		glview.h	\
		printable.h \
		print-setup-dlg.h	\
		spectrumdoc.h \
		spectrumview.h \
		ui-manager.h
