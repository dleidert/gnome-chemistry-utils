lib_LTLIBRARIES= libgcu.la

libgcu_la_SOURCES = \
		atom.cc \
		bond.cc \
		object.cc \
		element.cc \
		matrix.cc \
		xml-utils.cc \
		chemistry.cc

libgcu_la_headers = \
		atom.h \
		bond.h \
		object.h \
		element.h \
		matrix.h \
		xml-utils.h \
		chemistry.h
		
gcuincludedir = $(includedir)/gcu/chemistry
gcuinclude_HEADERS = $(libgcu_la_headers)
		
libgcu_la_LDFLAGS = -version-info  @GCU_VERSION_INFO@ @GCU_LIBS@
AM_CXXFLAGS = @GCU_CFLAGS@

xmldir = $(datadir)/gchemutils

xml_in_files = elements.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(xml_DATA)
