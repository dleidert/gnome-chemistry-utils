// -*- C++ -*-

/* 
 * CDXML files loader plugin
 * cdxml.cc 
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/objprops.h>

#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-libxml.h>
#include <glib/gi18n-lib.h>
#include <libintl.h>
#include <cstdio>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

using namespace std;
using namespace gcu;

static map<string, unsigned> DocProps, AtomProps, BondProps;

class CDXMLLoader: public gcu::Loader
{
public:
	CDXMLLoader ();
	virtual ~CDXMLLoader ();

	bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);
};

CDXMLLoader::CDXMLLoader ()
{
	AddMimeType ("chemical/x-cdxml");
	DocProps["BondLength"] = GCU_PROP_THEME_BOND_LENGTH;
	DocProps["Comment"] = GCU_PROP_DOC_COMMENT;
	DocProps["CreationDate"] = GCU_PROP_DOC_CREATION_TIME;
	DocProps["CreationUserName"] = GCU_PROP_DOC_CREATOR;
	DocProps["ModificationDate"] = GCU_PROP_DOC_MODIFICATION_TIME;
	DocProps["Name"] = GCU_PROP_DOC_TITLE;
	AtomProps["p"] = GCU_PROP_POS2D;
	AtomProps["Element"] = GCU_PROP_ATOM_Z;
	AtomProps["id"] = GCU_PROP_ID;
	BondProps["id"] = GCU_PROP_ID;
	BondProps["B"] = GCU_PROP_BOND_BEGIN;
	BondProps["DISPLAY"] = GCU_PROP_BOND_TYPE;
	BondProps["E"] = GCU_PROP_BOND_END;
	BondProps["Order"] = GCU_PROP_BOND_ORDER;
	BondProps["id"] = GCU_PROP_ID;
}

CDXMLLoader::~CDXMLLoader ()
{
	RemoveMimeType ("chemical/x-cdxml");
}

typedef struct {
	Object *obj;
	unsigned property;
	string value;
} CDXMLProps;

typedef struct {
	unsigned index;
	string encoding;
	string name;
} CDXMLFont;

typedef struct {
	Document *doc;
	IOContext *context;
	stack<Object*> cur;
	list<CDXMLProps> failed;
	map<unsigned, CDXMLFont> fonts;
	vector<string> colors;
} CDXMLReadState;

static void
cdxml_doc (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	map<string, unsigned>::iterator it;
	while (*attrs) {
		if ((it = DocProps.find ((char const *) *attrs++)) != DocProps.end ()) {
			state->doc->SetProperty ((*it).second, (char const *) *attrs);}
		attrs++;
	}
	state->cur.push (state->doc);
}

static void
cdxml_fragment_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
}

static void
cdxml_node_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("atom", state->cur.top ());
	obj->SetProperty (GCU_PROP_ATOM_Z, "6");
	map<string, unsigned>::iterator it;
	while (*attrs) {
		if ((it = AtomProps.find ((char const *) *attrs++)) != AtomProps.end ()) {
			obj->SetProperty ((*it).second, (char const *) *attrs);}
		attrs++;
	}
	state->cur.push (obj);
}

static map<string, int>BondTypes;
static void
cdxml_bond_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("bond", state->cur.top ());
	obj->SetProperty (GCU_PROP_BOND_ORDER, "1");
	map<string, unsigned>::iterator it;
	while (*attrs) {
		if ((it = BondProps.find ((char const *) *attrs++)) != BondProps.end ()) {
			if ((*it).second == GCU_PROP_BOND_TYPE) {
				if (BondTypes.empty ()) {
					BondTypes["Solid"] = 0;
					BondTypes["Dash"] = 1;
					BondTypes["Hash"] = 2;
					BondTypes["WedgedHashBegin"] = 3;
					BondTypes["WedgedHashEnd"] = 4;
					BondTypes["Bold"] = 5;
					BondTypes["WedgeBegin"] = 6;
					BondTypes["WedgeEnd"] = 7;
					BondTypes["Wavy"] = 8;
					BondTypes["HollowWedgeBegin"] = 9;
					BondTypes["HollowWedgeEnd"] = 10;
					BondTypes["WavyWedgeBegin"] = 11;
					BondTypes["WavyWedgeEnd"] = 12;
					BondTypes["Dot"] = 13;
					BondTypes["DashDot"] = 14;
				}
				switch (BondTypes[(char const *) *attrs]) {
				case 1:
				case 2:
				case 3:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "hash");
					break;
				case 4:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "hash-invert");
					break;
				case 5:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "large");
					break;
				case 6:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "wedge");
					break;
				case 7:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "wedge-invert");
					break;
				case 8:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "squiggle");
					break;
				default:
					obj->SetProperty (GCU_PROP_BOND_TYPE, "normal");
				}
			} else if ((*it).second == GCU_PROP_BOND_ORDER) {
				unsigned order = atoi ((char const *) *attrs);
				switch (order) {
				case 2:
					obj->SetProperty (GCU_PROP_BOND_ORDER, "2");
					break;
				case 4:
					obj->SetProperty (GCU_PROP_BOND_ORDER, "3");
					break;
				default:
					obj->SetProperty (GCU_PROP_BOND_ORDER, "1");
					break;
				}
			} else if (!obj->SetProperty ((*it).second, (char const *) *attrs)) {
				CDXMLProps p;
				p.obj = obj;
				p.property = (*it).second;
				p.value = (char const *) *attrs;
				state->failed.push_back (p);
			}
		}
		attrs++;
	}
	state->cur.push (obj);
}

static void
cdxml_simple_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->cur.top ()->Lock (false);
	state->cur.top ()->OnLoaded ();
	state->cur.pop ();
}

static void
cdxml_font_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	CDXMLFont font;
	font.index = 0;
	while (*attrs) {
		if (!strcmp ((char const *) *attrs, "id"))
			font.index = atoi ((char const *) *(attrs + 1));
		else if (!strcmp ((char const *) *attrs, "charset"))
			font.encoding = (char const *) *(attrs + 1);
		else if (!strcmp ((char const *) *attrs, "name"))
			font.name = (char const *) *(attrs + 1);
		attrs += 2;
	}
	state->fonts[font.index] = font;
}

static void
cdxml_color (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	string red, green, blue;
	while (*attrs) {
		if (!strcmp ((char const *) *attrs, "r"))
			red = (char const *) attrs[1];
		else if (!strcmp ((char const *) *attrs, "g"))
			green = (char const *) attrs[1];
		else if (!strcmp ((char const *) *attrs, "b"))
			blue = (char const *) attrs[1];
		attrs += 2;
	}
	state->colors.push_back (red + " " + green + " " + blue);
}

static void
cdxml_group_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("group", state->cur.top ());
	obj->Lock ();
	state->cur.push (obj);
}

////////////////////////////////////////////////////////////////////////////////
// Reading code
static GsfXMLInNode const cdxml_dtd[] = {
GSF_XML_IN_NODE (CDXML, CDXML, -1, "CDXML", GSF_XML_CONTENT, &cdxml_doc, NULL),
	GSF_XML_IN_NODE (CDXML, COLORTABLE, -1, "colortable", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (COLORTABLE, COLOR, -1, "color", GSF_XML_CONTENT, &cdxml_color, NULL),
	GSF_XML_IN_NODE (CDXML, FONTTABLE, -1, "fonttable", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (FONTTABLE, FONT, -1, "font", GSF_XML_CONTENT, cdxml_font_start, NULL),
	GSF_XML_IN_NODE (CDXML, PAGE, -1, "page", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, T, -1, "t", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, FRAGMENT, -1, "fragment", GSF_XML_CONTENT, &cdxml_fragment_start, &cdxml_simple_end),
			GSF_XML_IN_NODE (FRAGMENT, NODE, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
			GSF_XML_IN_NODE (FRAGMENT, BOND, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
			GSF_XML_IN_NODE (FRAGMENT, T1, -1, "t", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GROUP, -1, "group", GSF_XML_CONTENT, cdxml_group_start, cdxml_simple_end),
			GSF_XML_IN_NODE (GROUP, FRAGMENT1, -1, "fragment", GSF_XML_CONTENT, cdxml_fragment_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, NODE1, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, BOND1, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, T11, -1, "t", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GRAPHIC, -1, "graphic", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, ALTGROUP, -1, "altgroup", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, CURVE, -1, "curve", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, STEP, -1, "step", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, SCHEME, -1, "scheme", GSF_XML_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (SCHEME, REACTIONSTEP, -1, "step", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, SPECTRUM, -1, "spectrum", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, EMBEDDEDOBJECT, -1, "embeddedobject", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, SEQUENCE, -1, "sequence", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, CROSSREFERENCE, -1, "crossreference", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, SPLITTER, -1, "splitter", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, TABLE, -1, "table", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, BRACKETEDGROUP, -1, "bracketedgroup", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, BORDER, -1, "border", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GEOMETRY, -1, "geometry", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, CONSTRAINT, -1, "constraint", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, TLCPLATE, -1, "tlcplate", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, CHEMICALPROPERTY, -1, "chemicalproperty", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, ARROW, -1, "arrow", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, BIOSHAPE, -1, "bioshape", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, STOICHIOMETRY, -1, "stoichiometrygrid", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, PLASMIDMAP, -1, "plasmidmap", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, OBJECTTAG, -1, "objecttag", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, ANNOTATION, -1, "annotation", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, RLOGIC, -1, "rlogic", GSF_XML_CONTENT, NULL, NULL),
	GSF_XML_IN_NODE (CDXML, TEMPLATEGRID, -1, "templategrid", GSF_XML_CONTENT, NULL, NULL),
GSF_XML_IN_NODE_END
};

bool CDXMLLoader::Read  (Document *doc, GsfInput *in, char const *mime_type, IOContext *io)
{
	CDXMLReadState state;

	state.doc = doc;
	state.context = io;
	bool  success = false;
	state.colors.push_back ("1 1 1"); // white
	state.colors.push_back ("0 0 0"); // black

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cdxml_dtd, NULL);
		success = gsf_xml_in_doc_parse (xml, in, &state);

		if (!success)
			gnm_io_warning (state.context,
				_("'%s' is corrupt!"),
				gsf_input_name (in));
		else if (!state.failed.empty ()) {
			CDXMLProps p;
			Object *parent = NULL;
			while (!state.failed.empty ()) {
				p = state.failed.front ();
				if (parent != p.obj->GetParent ()) {
					if (parent)
						parent->OnLoaded ();
						parent = p.obj->GetParent ();
				}
				if (!p.obj->SetProperty (p.property, p.value.c_str ())) {
					success = false;
					gnm_io_warning (state.context,
						_("'%s' is corrupt!"),
						gsf_input_name (in));
				}
				state.failed.pop_front ();
			}
			if (parent)
				parent->OnLoaded ();
		}
		gsf_xml_in_doc_free (xml);
	}
	return success;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CDXMLLoader::Write  (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io)
{
	map<string, CDXMLFont> fonts;

	if (NULL != out) {
		GsfXMLOut *xml = gsf_xml_out_new (out);
		gsf_xml_out_set_doc_type (xml, "<!DOCTYPE CDXML SYSTEM \"http://www.cambridgesoft.com/xml/cdxml.dtd\">");
		gsf_xml_out_start_element (xml, "CDXML");
		gsf_xml_out_end_element (xml);
		g_object_unref (xml);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static CDXMLLoader loader;

extern "C" {

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

G_MODULE_EXPORT void
go_plugin_init (GOPlugin *plugin, GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
#ifdef ENABLE_NLS
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
}

G_MODULE_EXPORT void
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
}

}
