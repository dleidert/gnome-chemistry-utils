EXTRA_DIST = $(xml_in_files)

CLEANFILES = $(xml_DATA)

MAINTAINERCLEANFILES = Makefile.in

xml_in_files = \
	elements.xml.in \
	elecprops.xml.in \
	isotopes.xml.in \
	radii.xml.in \
	residues.xml.in \
	space-groups.xml.in \
	databases.xml.in

xmldir = $(datadir)/gchemutils/@GCU_API_VER@
xml_DATA = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@
