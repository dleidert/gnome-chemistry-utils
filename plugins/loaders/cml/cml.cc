// -*- C++ -*-

/* 
 * CML files loader plugin
 * cml.cc 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/molecule.h>
#include <gcu/objprops.h>

#include <goffice/app/module-plugin-defs.h>
#include <gsf/gsf-libxml.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <libintl.h>

using namespace gcu;
using namespace std;

static map<string, unsigned> KnownProps;

typedef struct {
	Document *doc;
	IOContext *context;
	stack<Object*> cur;
	ContentType type;
	string curstr;
} CMLReadState;

typedef struct {
} CMLWriteState;

class CMLLoader: public Loader
{
public:
	CMLLoader ();
	virtual ~CMLLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Object *obj, GsfOutput *out, char const *mime_type, IOContext *io, ContentType type);

	bool WriteObject (GsfXMLOut *xml, Object *object, IOContext *io, ContentType type);

private:
	map <string, bool (*) (CMLLoader *, GsfXMLOut *, Object *, IOContext *s, ContentType)> m_WriteCallbacks;
};

////////////////////////////////////////////////////////////////////////////////
// Write callbacks

bool cml_write_atom (G_GNUC_UNUSED CMLLoader *loader, GsfXMLOut *xml, Object *object, G_GNUC_UNUSED IOContext *io, ContentType type)
{
	gsf_xml_out_start_element (xml, "atom");
	gsf_xml_out_add_cstr_unchecked (xml, "id", object->GetId ());
	string prop;
	prop = object->GetProperty (GCU_PROP_ATOM_SYMBOL);
	gsf_xml_out_add_cstr_unchecked (xml, "elementType", prop.c_str ());
	prop = object->GetProperty (GCU_PROP_ATOM_CHARGE);
	if (prop != "0")
		gsf_xml_out_add_cstr_unchecked (xml, "formalCharge", prop.c_str ());
	if (type == ContentType2D) {
		double x, y;
		prop = object->GetProperty (GCU_PROP_POS2D);
		if (prop.length ()) {
			sscanf (prop.c_str (), "%lg %lg", &x, &y);
			gsf_xml_out_add_float (xml, "x2", x, -1);
			gsf_xml_out_add_float (xml, "y2", -y, -1); // reverse y order
		}
	} else {
		double x, y, z;
		prop = object->GetProperty (GCU_PROP_POS3D);
		if (prop.length ()) {
			sscanf (prop.c_str (), "%lg %lg %lg", &x, &y, &z);
			gsf_xml_out_add_float (xml, "x3", x, -1);
			gsf_xml_out_add_float (xml, "y3", y, -1);
			gsf_xml_out_add_float (xml, "z3", z, -1);
		}
	}
	gsf_xml_out_end_element (xml);
	return true;
}

bool cml_write_bond (G_GNUC_UNUSED CMLLoader *loader, GsfXMLOut *xml, Object *object, G_GNUC_UNUSED IOContext *io, G_GNUC_UNUSED ContentType type)
{
	gsf_xml_out_start_element (xml, "bond");
	gsf_xml_out_add_cstr_unchecked (xml, "id", object->GetId ());
	string prop = object->GetProperty (GCU_PROP_BOND_BEGIN) + " " + object->GetProperty (GCU_PROP_BOND_END);
	gsf_xml_out_add_cstr_unchecked (xml, "atomRefs2", prop.c_str ());
	prop = object->GetProperty (GCU_PROP_BOND_ORDER);
	gsf_xml_out_add_cstr_unchecked (xml, "order", prop.c_str ());
	prop = object->GetProperty (GCU_PROP_BOND_TYPE);
	if (prop == "wedge") {
		gsf_xml_out_start_element (xml, "bondStereo");
		gsf_xml_out_add_cstr_unchecked (xml, NULL, "W");
		gsf_xml_out_end_element (xml);
	} else if (prop == "hash") {
		gsf_xml_out_start_element (xml, "bondStereo");
		gsf_xml_out_add_cstr_unchecked (xml, NULL, "H");
		gsf_xml_out_end_element (xml);
	}
	gsf_xml_out_end_element (xml);
	return true;
}

bool cml_write_molecule (CMLLoader *loader, GsfXMLOut *xml, Object *object, IOContext *io, ContentType type)
{
	gsf_xml_out_start_element (xml, "molecule");
	std::map <std::string, Object *>::iterator i;
	Object *child = object->GetFirstChild (i);
	list <Object *> bonds, fragments;
	gsf_xml_out_start_element (xml, "atomArray");
	while (child) {
		switch (child->GetType ()) {
		case AtomType:
			loader->WriteObject (xml, child, io, type);
			break;
		case BondType:
				bonds.push_back (child);
			break;
		default:
			break;
		}
		child = object->GetNextChild (i);
	}
	gsf_xml_out_end_element (xml);
	// now save bonds
	if (bonds.size () > 0) {
		gsf_xml_out_start_element (xml, "bondArray");
		list <Object *>::iterator it, end = bonds.end ();
		for (it = bonds.begin (); it != end; it++)
			loader->WriteObject (xml, *it, io, type);
		gsf_xml_out_end_element (xml);
	}
	gsf_xml_out_end_element (xml);
	return true;
}

CMLLoader::CMLLoader ()
{
	AddMimeType ("chemical/x-cml");
	KnownProps["title"] = GCU_PROP_DOC_TITLE;
	// general properties
	KnownProps["id"] = GCU_PROP_ID;
	KnownProps["x2"] = GCU_PROP_X;
	KnownProps["y2"] = GCU_PROP_Y;
	KnownProps["x3"] = GCU_PROP_X;
	KnownProps["y3"] = GCU_PROP_Y;
	KnownProps["z3"] = GCU_PROP_Z;
	// atom properties
	KnownProps["elementType"] = GCU_PROP_ATOM_SYMBOL;
	KnownProps["formalCharge"] = GCU_PROP_ATOM_CHARGE;
	// bond properties
	KnownProps["order"] = GCU_PROP_BOND_ORDER;

	// Add write callbacks
	m_WriteCallbacks["atom"] = cml_write_atom;
	m_WriteCallbacks["bond"] = cml_write_bond;
	m_WriteCallbacks["molecule"] = cml_write_molecule;
}

CMLLoader::~CMLLoader ()
{
	RemoveMimeType ("chemical/x-cml");
}
////////////////////////////////////////////////////////////////////////////////
// Reading code

static void
cml_simple_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	state->cur.top ()->Lock (false);
	state->cur.top ()->OnLoaded ();
	state->cur.pop ();
}

static void
cml_doc (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	map<string, unsigned>::iterator it;
	if (attrs)
		while (*attrs) {
			if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
				state->doc->SetProperty ((*it).second, (char const *) *attrs);}
			attrs++;
		}
	state->cur.push (state->doc);
}

////////////////////////////////////////////////////////////////////////////////
// Molecule name if any

static void
cml_mol_name_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	while (*attrs) {
		if (!strcmp ((char const *) *attrs, "convention")) {
			attrs++;
			state->curstr = reinterpret_cast <char const *> (*attrs);
		} else
			attrs++;
		attrs++;
	}
	
}

static void
cml_mol_name_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	static_cast <Molecule *> (state->cur.top ())->SetName (xin->content->str, state->curstr.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Atom code

static void
cml_atom_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("atom", state->cur.top ());
	obj->SetProperty (GCU_PROP_ATOM_SYMBOL, "C");
	map <string, unsigned>::iterator it;
	if (attrs)
		while (*attrs) {
			if (state->type == ContentTypeMisc) {
				if (!strcmp ((char const *) *attrs, "x2"))
					state->type = ContentType2D;
				else if (!strcmp ((char const *) *attrs, "x3"))
					state->type = ContentType3D;
			}
			if (!strcmp ((char const *) *attrs, "y2")) {
				attrs++;
				double x = -strtod ((char const *) *attrs, NULL); // reverse y direction, see comment in cml_write_atom
				ostringstream res;
				res << x;
				obj->SetProperty (GCU_PROP_Y, res.str ().c_str ());
			} else if ((it = KnownProps.find ((char const *) *attrs)) != KnownProps.end ()) {
				attrs++;
				obj->SetProperty ((*it).second, (char const *) *attrs);
			}
			attrs++;
		}
}

////////////////////////////////////////////////////////////////////////////////
// Bond code

static void
cml_bond_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("bond", state->cur.top ());
	map <string, unsigned>::iterator it;
	if (attrs)
		while (*attrs) {
			if ((it = KnownProps.find ((char const *) *attrs)) != KnownProps.end ()) {
				attrs++;
				obj->SetProperty ((*it).second, (char const *) *attrs);
			} else if (!strcmp ((char const *) *attrs, "atomRefs2")) {
				attrs++;
				char **atom_ids = g_strsplit ((char const *) *attrs, " ", 2);
				obj->SetProperty (GCU_PROP_BOND_BEGIN, atom_ids[0]);
				obj->SetProperty (GCU_PROP_BOND_END, atom_ids[1]);
				g_strfreev (atom_ids);
			} else
				attrs++;
			attrs++;
		}
	state->cur.push (obj);
}

static void
cml_bond_stereo (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	string stereo = xin->content->str;
	if (stereo == "W")
		state->cur.top ()->SetProperty (GCU_PROP_BOND_TYPE, "wedge");
	else if (stereo == "H")
		state->cur.top ()->SetProperty (GCU_PROP_BOND_TYPE, "hash");
};
////////////////////////////////////////////////////////////////////////////////
// Molecule code
	
static void
cml_mol_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	static GsfXMLInNode const mol_dtd[] = {
	GSF_XML_IN_NODE (MOL, MOL, -1, "molecule", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL, MOL_NAME, -1, "name", GSF_XML_CONTENT, cml_mol_name_start, cml_mol_name_end),
		GSF_XML_IN_NODE (MOL, MOL_IDENTIFIER, -1, "identifier", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL, MOL_FORMULA, -1, "formula", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL, MOL_PROPS, -1, "propertyList", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL_PROPS, MOL_PROP, -1, "property", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL_PROP, MOL_PROP_SCALAR, -1, "scalar", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL, ATOM_ARRAY, -1, "atomArray", GSF_XML_NO_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (ATOM_ARRAY, ATOM, -1, "atom", GSF_XML_NO_CONTENT, cml_atom_start, NULL),
		GSF_XML_IN_NODE (MOL, BOND_ARRAY, -1, "bondArray", GSF_XML_NO_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (BOND_ARRAY, BOND, -1, "bond", GSF_XML_NO_CONTENT, cml_bond_start, cml_simple_end),
				GSF_XML_IN_NODE (BOND, BOND_STEREO, -1, "bondStereo", GSF_XML_CONTENT, NULL, cml_bond_stereo),
	GSF_XML_IN_NODE_END
	};
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
	static GsfXMLInDoc *doc = NULL;
	if (NULL == doc)
		doc = gsf_xml_in_doc_new (mol_dtd, NULL);
	gsf_xml_in_push_state (xin, doc, state, (GsfXMLInExtDtor) NULL, attrs);
}

////////////////////////////////////////////////////////////////////////////////
// Reading code
static GsfXMLInNode const cml_dtd[] = {
GSF_XML_IN_NODE (CML, CML, -1, "cml", GSF_XML_CONTENT, &cml_doc, NULL),
	GSF_XML_IN_NODE (CML, MOLECULE, -1, "molecule", GSF_XML_CONTENT, cml_mol_start, cml_simple_end),
};

ContentType CMLLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io)
{
	CMLReadState state;
	bool  success = false;

	state.doc = doc;
	state.context = io;
	state.cur.push (doc);
	state.type = ContentTypeMisc;

	doc->SetScale (100.);

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cml_dtd, NULL);
		success = gsf_xml_in_doc_parse (xml, in, &state);

		if (!success)
			gnm_io_warning (state.context,
				_("'%s' is corrupt!"),
				gsf_input_name (in));
		gsf_xml_in_doc_free (xml);
	}
	return success? state.type: ContentTypeUnknown;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CMLLoader::WriteObject (GsfXMLOut *xml, Object *object, IOContext *io, ContentType type)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CMLLoader *, GsfXMLOut *, Object *, IOContext *, ContentType)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, xml, object, io, type);
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

bool CMLLoader::Write  (Object *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, IOContext *io, ContentType type)
{
	if (NULL != out) {
		GsfXMLOut *xml = gsf_xml_out_new (out);
		gsf_xml_out_start_element (xml, "cml");
		gsf_xml_out_add_cstr_unchecked (xml, "xmlns:cml", "http://www.xml-cml.org/schema");
		// FIXME: add other namespaces if needed
		Document *doc = dynamic_cast <Document *> (obj);
		if (doc) {
			doc->SetScale (100);
			string title = doc->GetProperty (GCU_PROP_DOC_TITLE);
			if (title.length ())
				gsf_xml_out_add_cstr (xml, "title", title.c_str ());
			std::map <std::string, Object *>::iterator i;
			Object *child = doc->GetFirstChild (i);
			while (child) {
				if (!WriteObject (xml, child, io, type)) {
					g_object_unref (xml);
					return false;
				}
				child = doc->GetNextChild (i);
			}
		} else
			WriteObject (xml, obj, io, type);
		gsf_xml_out_end_element (xml);
		g_object_unref (xml);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Initialization

static CMLLoader loader;

extern "C" {

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

G_MODULE_EXPORT void
go_plugin_init (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}

G_MODULE_EXPORT void
go_plugin_shutdown (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
}

}
