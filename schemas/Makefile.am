MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = $(gsettings_SCHEMAS)

gsettings_SCHEMAS =	\
	org.gnome.gchemutils.gschema.xml	\
	org.gnome.gchemutils.crystal.gschema.xml	\
	org.gnome.gchemutils.paint.gschema.xml
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

EXTRA_DIST = \
	org.gnome.gchemutils.gschema.xml.in.in	\
	org.gnome.gchemutils.crystal.gschema.xml.in.in	\
	org.gnome.gchemutils.paint.gschema.xml.in.in