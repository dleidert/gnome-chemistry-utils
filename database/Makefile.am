EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

xmldir = $(datadir)/gchemutils
xml_in_files = elements.xml.in elecprops.xml.in isotopes.xml.in radii.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

