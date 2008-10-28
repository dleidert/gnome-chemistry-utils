AUTOMAKE_OPTIONS = -Wno-portability

EXTRA_DIST = $(DOC_MODULE).omf.in C/$(DOC_MODULE).xml

MAINTAINERCLEANFILES = Makefile.in $(EXTRA_DIST)

include $(top_srcdir)/gnome-doc-utils.make
dist-hook: doc-dist-hook

DOC_PROGRAM = gchemcalc

DOC_MODULE = $(shell echo "$(DOC_PROGRAM)" | sed '$(transform)')
DOC_ENTITIES = legal.xml
DOC_INCLUDES = 

DOC_LINGUAS =

if TRANSFORM_EXISTS
include $(top_srcdir)/Makefile.incl
endif
