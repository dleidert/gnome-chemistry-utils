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
	fragmenttool.cc \
	texttool.cc

noinst_HEADERS = \
	plugin.h \
	fragmenttool.h \
	texttool.h

if GCU_WITH_LASEM
text_la_SOURCES += \
	equation.cc \
	mathtool.cc
noinst_HEADERS += \
	equation.h \
	mathtool.h
AM_CPPFLAGS += \
	$(lasem_CFLAGS)
text_la_LIBADD += \
	$(lasem_LIBS)
endif

uidir = $(datadir)/gchemutils/@GCU_API_VER@/ui/paint/plugins/text
dist_ui_DATA = fontsel.ui eq-props.ui
