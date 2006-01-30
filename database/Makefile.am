xmldir = $(datadir)/gchemutils

xml_in_files = \
	elements.xml.in \
	elecprops.xml.in \
	isotopes.xml.in \
	radii.xml.in

xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

CLEANFILES = \
	elements.xml \
	elecprops.xml \
	isotopes.xml \
	radii.xml

EXTRA_DIST=$(xml_in_files)
