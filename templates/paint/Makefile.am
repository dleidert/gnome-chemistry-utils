xmldir = $(datadir)/gchemutils-unstable/paint/templates

xml_in_files = templates.xml.in
xml_DATA = $(xml_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(xml_in_files) $(xml_DATA)

CLEANFILES = $(xml_DATA)
