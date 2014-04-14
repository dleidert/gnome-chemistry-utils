EXTRA_DIST = $(ui_DATA)

MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
		-I$(top_srcdir) -I$(top_srcdir)/libs \
		$(goffice_CFLAGS) \
		$(GCU_CFLAGS)

DEFS += -DDATADIR=\"$(datadir)\" \
        -DUIDIR=\"$(uidir)\" \
        -DPLUGINSDIR=\"$(textdir)\"

textdir = $(libdir)/gchemutils/@GCU_API_VER@/plugins/paint
text_LTLIBRARIES = text.la

text_la_LDFLAGS = -module -avoid-version
text_la_LIBADD = \
		$(gtk_LIBS) $(goffice_LIBS) $(xml_LIBS) \
		$(top_builddir)/libs/gccv/libgccv-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcu/libgcu-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcugtk/libgcugtk-@GCU_API_VER@.la \
		$(top_builddir)/libs/gcp/libgcp-@GCU_API_VER@.la

text_la_SOURCES = \
	plugin.cc \
	equation.cc \
	fragmenttool.cc \
	mathtool.cc \
	texttool.cc

noinst_HEADERS = \
	plugin.h \
	equation.h \
	fragmenttool.h \
	mathtool.h \
	texttool.h

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/text
ui_DATA = fontsel.ui eq-props.ui
