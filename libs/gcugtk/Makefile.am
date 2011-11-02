MAINTAINERCLEANFILES = Makefile.in

INCLUDES = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)
libgcugtk_@GCU_API_VER@_la_LDFLAGS = \
		-version-info  @GCU_VERSION_INFO@
libgcugtk_@GCU_API_VER@_la_LIBADD = \
		$(goffice_LIBS) \
		$(gsf_LIBS) \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		-lX11
DEFS += -DGCU_API_VER=\"@GCU_API_VER@\" \
		-DDATADIR=\"$(datadir)\" \
		-DLOCALEDIR=\"$(localedir)\" \
		-DLIBEXECDIR=\"$(libexecdir)\" \
		-DPKGDATADIR=\"$(datadir)/gchemutils/@GCU_API_VER@\" \
		-DUIDIR=\"$(datadir)/gchemutils/@GCU_API_VER@/ui/libgcu\" \
		-DGCU_PLUGINS_DIR=\"$(libdir)/gchemutils/@GCU_API_VER@/plugins\"
lib_LTLIBRARIES= libgcugtk-@GCU_API_VER@.la

libgcugtk_@GCU_API_VER@_la_SOURCES = \
		marshalers.list	\
		\
		application.cc \
		cmd-context-gtk.cc \
		chem3dapplication.cc	\
		chem3ddoc.cc	\
		chem3dview.cc	\
		chem3dwindow.cc	\
		dialog.cc	\
		filechooser.cc	\
		gcuchem3dviewer.cc \
		gcucomboperiodic.c \
		gcuperiodic.c \
		gcuspectrumviewer.cc \
		glview.cc	\
		message.cc	\
		molecule.cc \
		printable.cc \
		print-setup-dlg.cc	\
		spectrumdoc.cc \
		spectrumview.cc \
		stringdlg.cc \
		stringinputdlg.cc \
		ui-builder.cc \
		ui-manager.cc \
		window.cc

noinst_HEADERS = \
		application.h \
		cmd-context-gtk.h \
		chem3dapplication.h	\
		chem3ddoc.h	\
		chem3dview.h	\
		chem3dwindow.h	\
		dialog.h	\
		filechooser.h	\
		gcuchem3dviewer.h \
		gcucomboperiodic.h \
		gcuperiodic.h \
		gcuspectrumviewer.h \
		glview.h	\
		message.h	\
		molecule.h \
		printable.h \
		print-setup-dlg.h	\
		spectrumdoc.h \
		spectrumview.h \
		stringdlg.h \
		stringinputdlg.h \
		ui-builder.h \
		ui-manager.h \
		window.h

CLEANFILES =			\
	marshalers.h		\
	marshalers.c

# A hint is needed to build the header first:
BUILT_SOURCES = marshalers.h

# Another hint, see gnome bugs #172211 and #172212:
non-intermediate: marshalers.c

GENMARSHAL_COMMAND = $(GLIB_GENMARSHAL) --prefix=gcu_

.list.h:
	$(GENMARSHAL_COMMAND) --header $< >$@

.list.c:
	(echo '/* This file has been automatically generated.  Do not edit. */' && \
	echo '#include <config.h>' && \
	echo '#include "$*.h"' && \
	$(GENMARSHAL_COMMAND) --body $< ) >$@
