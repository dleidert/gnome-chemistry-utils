EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

xml_in_files = templates.xml.in haworth.xml.in

xmldir = $(pkgdatadir)/paint/templates
xml_DATA = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@
