// -*- C++ -*-

/*
 * CML files loader plugin
 * cml.cc
 *
 * Copyright (C) 2008-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/application.h>
#include <gcu/document.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <gcu/spacegroup.h>
#include <gcu/transform3d.h>

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
	Application *app;
	GOIOContext *context;
	stack<Object*> cur;
	ContentType type;
	string curstr;
	string proptype;
	unsigned cur_prop;
	gpointer data; // used for whatever has to be stores which is not an Object
} CMLReadState;

typedef struct {
} CMLWriteState;

class CMLLoader: public Loader
{
public:
	CMLLoader ();
	virtual ~CMLLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
	bool Write (Object const *obj, GsfOutput *out, char const *mime_type, GOIOContext *io, ContentType type);

	bool WriteObject (GsfXMLOut *xml, Object const *object, GOIOContext *io, ContentType type);

private:
	map <string, bool (*) (CMLLoader *, GsfXMLOut *, Object const *, GOIOContext *s, ContentType)> m_WriteCallbacks;
};

////////////////////////////////////////////////////////////////////////////////
// Write callbacks

bool cml_write_atom (G_GNUC_UNUSED CMLLoader *loader, GsfXMLOut *xml, Object const *object, G_GNUC_UNUSED GOIOContext *io, ContentType type)
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
			istringstream in (prop);
			in >> x >> y;
			gsf_xml_out_add_float (xml, "x2", x, -1);
			gsf_xml_out_add_float (xml, "y2", -y, -1); // reverse y order
		}
	} else if (type == ContentTypeCrystal) {
		prop = object->GetProperty (GCU_PROP_XFRACT);
		gsf_xml_out_add_cstr_unchecked (xml, "xFract", prop.c_str ());
		prop = object->GetProperty (GCU_PROP_YFRACT);
		gsf_xml_out_add_cstr_unchecked (xml, "yFract", prop.c_str ());
		prop = object->GetProperty (GCU_PROP_ZFRACT);
		gsf_xml_out_add_cstr_unchecked (xml, "zFract", prop.c_str ());
	} else {
		double x, y, z;
		prop = object->GetProperty (GCU_PROP_POS3D);
		if (prop.length ()) {
			istringstream in (prop);
			in >> x >> y >> z;
			gsf_xml_out_add_float (xml, "x3", x, -1);
			gsf_xml_out_add_float (xml, "y3", y, -1);
			gsf_xml_out_add_float (xml, "z3", z, -1);
		}
	}
	gsf_xml_out_end_element (xml);
	return true;
}

bool cml_write_bond (G_GNUC_UNUSED CMLLoader *loader, GsfXMLOut *xml, Object const *object, G_GNUC_UNUSED GOIOContext *io, G_GNUC_UNUSED ContentType type)
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

bool cml_write_molecule (CMLLoader *loader, GsfXMLOut *xml, Object const *object, GOIOContext *io, ContentType type)
{
	gsf_xml_out_start_element (xml, "molecule");
	std::map <std::string, Object *>::const_iterator i;
	Object const *child = object->GetFirstChild (i);
	list <Object const *> bonds, fragments;
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
		list <Object const *>::iterator it, end = bonds.end ();
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
	KnownProps["xFract"] = GCU_PROP_XFRACT;
	KnownProps["yFract"] = GCU_PROP_YFRACT;
	KnownProps["zFract"] = GCU_PROP_ZFRACT;
	// bond properties
	KnownProps["order"] = GCU_PROP_BOND_ORDER;
	// crystal properties
	KnownProps["a"] = GCU_PROP_CELL_A;
	KnownProps["b"] = GCU_PROP_CELL_B;
	KnownProps["c"] = GCU_PROP_CELL_C;
	KnownProps["alpha"] = GCU_PROP_CELL_ALPHA;
	KnownProps["beta"] = GCU_PROP_CELL_BETA;
	KnownProps["gamma"] = GCU_PROP_CELL_GAMMA;
	// Add write callbacks
	m_WriteCallbacks["atom"] = cml_write_atom;
	m_WriteCallbacks["bond"] = cml_write_bond;
	m_WriteCallbacks["molecule"] = cml_write_molecule;
	// CIF derived properties
	KnownProps["iucr:_publ_contact_author_name"] = GCU_PROP_DOC_CREATOR;
	KnownProps["iucr:_publ_author_name"] = GCU_PROP_DOC_CREATOR;
	KnownProps["iucr:_publ_contact_author_email"] = GCU_PROP_DOC_CREATOR_EMAIL;
	KnownProps["iucr:_publ_author_email"] = GCU_PROP_DOC_CREATOR_EMAIL;

	KnownProps["iucr:_cell_length_a"] = GCU_PROP_CELL_A;
	KnownProps["iucr:_cell_length_b"] = GCU_PROP_CELL_B;
	KnownProps["iucr:_cell_length_c"] = GCU_PROP_CELL_C;
	KnownProps["iucr:_cell_angle_apha"] = GCU_PROP_CELL_ALPHA;
	KnownProps["iucr:_cell_angle_beta"] = GCU_PROP_CELL_BETA;
	KnownProps["iucr:_cell_angle_gamma"] = GCU_PROP_CELL_GAMMA;

	KnownProps["iucr:_chemical_name_common"] = GCU_PROP_CHEMICAL_NAME_COMMON;
	KnownProps["iucr:_chemical_name_systematic"] = GCU_PROP_CHEMICAL_NAME_SYSTEMATIC;
	KnownProps["iucr:_chemical_name_mineral"] = GCU_PROP_CHEMICAL_NAME_COMMON;
	KnownProps["iucr:_chemical_name_structure_type"] = GCU_PROP_CHEMICAL_NAME_STRUCTURE;

	KnownProps["iucr:_atom_site_fract_x"] = GCU_PROP_XFRACT;
	KnownProps["iucr:_atom_site_fract_y"] = GCU_PROP_YFRACT;
	KnownProps["iucr:_atom_site_fract_z"] = GCU_PROP_ZFRACT;
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
	if (state->cur.top ()) {
		state->cur.top ()->Lock (false);
		state->cur.top ()->OnLoaded ();
	}
	state->cur.pop ();
}

static void
cml_scalar_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	state->curstr = "";
	state->proptype = "xsd:double";
	if (attrs)
		while (*attrs) {
			if (!strcmp (reinterpret_cast <char const *> (*attrs), "title") ||
			    !strcmp (reinterpret_cast <char const *> (*attrs), "dictRef")) {
				map <string, unsigned>::iterator it = KnownProps.find (reinterpret_cast <char const *> (attrs[1]));
			    state->cur_prop = it == KnownProps.end ()? static_cast <unsigned> (GCU_PROP_MAX): (*it).second;
			} else if (!strcmp (reinterpret_cast <char const *> (*attrs), "dataType"))
				state->proptype = reinterpret_cast <char const *> (attrs[1]);
			else if (!strcmp (reinterpret_cast <char const *> (*attrs), "units"))
				state->curstr = reinterpret_cast <char const *> (attrs[1]);
			attrs += 2;
		}
}

static void
cml_scalar_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	if (state->proptype == "xsd:double") {
		double val = g_ascii_strtod (xin->content->str, NULL);
		if (state->curstr == "units:angstrom" || state->curstr == "")
			val *= 100.;
		char buf[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, val);
		state->doc->SetProperty (state->cur_prop, buf);
		g_free (buf);
	} else if (state->proptype == "xsd:string")
		state->doc->SetProperty (state->cur_prop, xin->content->str);
};

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
	Object *parent = state->cur.top ();
	Object *obj = state->app->CreateObject ("atom", parent? parent: state->doc);
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
				double x = -g_ascii_strtod ((char const *) *attrs, NULL); // reverse y direction, see comment in cml_write_atom
				ostringstream res;
				res << x;
				obj->SetProperty (GCU_PROP_Y, res.str ().c_str ());
			} else if ((it = KnownProps.find ((char const *) *attrs)) != KnownProps.end ()) {
				attrs++;
				obj->SetProperty ((*it).second, (char const *) *attrs);
			}
			attrs++;
		}
	state->cur.push (obj);
	state->doc->ObjectLoaded (obj);
}

static void
cml_atom_parity_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	if (attrs)
		while (*attrs) {
			if (!strcmp (reinterpret_cast < char const * > (*attrs), "atomRefs4")) {
				attrs++;
				state->curstr =  reinterpret_cast < char const * > (*attrs);
			} else
				attrs++;
			attrs++;
		}

}

static void
cml_atom_parity_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	string prop = xin->content->str;
	prop += ' ';
	prop += state->curstr;
	state->cur.top ()->SetProperty (GCU_PROP_ATOM_PARITY, prop.c_str ());
}

////////////////////////////////////////////////////////////////////////////////
// Bond code

static void
cml_bond_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	Object *obj = state->app->CreateObject ("bond", state->cur.top ());
	if (obj) {
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
	}
	state->cur.push (obj);
	state->doc->ObjectLoaded (obj);
}

static void
cml_bond_stereo (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	if (!state->cur.top ())
		return;
	string stereo = xin->content->str;
	if (stereo == "W")
		state->cur.top ()->SetProperty (GCU_PROP_BOND_TYPE, "wedge");
	else if (stereo == "H")
		state->cur.top ()->SetProperty (GCU_PROP_BOND_TYPE, "hash");
};

////////////////////////////////////////////////////////////////////////////////
// Crystal code

static void
cml_crystal_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	state->type = ContentTypeCrystal;
	state->data = new SpaceGroup ();

	state->doc->SetScale (1.); // FIXME: assuming fractional coordinates
}

static void
cml_crystal_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	SpaceGroup *group = static_cast <SpaceGroup *> (state->data);
	SpaceGroup const *found = SpaceGroup::Find (group);
	if (found != NULL)
		state->doc->SetProperty (GCU_PROP_SPACE_GROUP, found->GetHallName ().c_str ());
	delete group;
	state->data = NULL;
};

static void
cml_symmetry_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	SpaceGroup *group = static_cast <SpaceGroup *> (state->data);
	if (attrs)
		while (*attrs) {
			if (!strcmp (reinterpret_cast <char const *> (*attrs), "spaceGroup")) {
				SpaceGroup const *found = SpaceGroup::GetSpaceGroup (reinterpret_cast <char const *> (attrs[1]));
				if (found)
					group->SetHallName (found->GetHallName ());
			}
			attrs += 2;
		}
}

static void
cml_transform_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CMLReadState *state = (CMLReadState *) xin->user_state;
	SpaceGroup *group = static_cast <SpaceGroup *> (state->data);
	group->AddTransform (xin->content->str);
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
		GSF_XML_IN_NODE (MOL_PROP, MOL_PROP_SCALAR, -1, "scalar", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (MOL, ATOM_ARRAY, -1, "atomArray", GSF_XML_NO_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (ATOM_ARRAY, ATOM, -1, "atom", GSF_XML_NO_CONTENT, cml_atom_start, cml_simple_end),
				GSF_XML_IN_NODE (ATOM, ATOM_PARITY, -1, "atomParity", GSF_XML_CONTENT, cml_atom_parity_start, cml_atom_parity_end),
		GSF_XML_IN_NODE (MOL, BOND_ARRAY, -1, "bondArray", GSF_XML_NO_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (BOND_ARRAY, BOND, -1, "bond", GSF_XML_NO_CONTENT, cml_bond_start, cml_simple_end),
				GSF_XML_IN_NODE (BOND, BOND_STEREO, -1, "bondStereo", GSF_XML_CONTENT, NULL, cml_bond_stereo),
		GSF_XML_IN_NODE (MOL, CRYSTAL, -1, "crystal", GSF_XML_NO_CONTENT, cml_crystal_start, cml_crystal_end),
			GSF_XML_IN_NODE (CRYSTAL, CRYSTAL_SCALAR, -1, "scalar", GSF_XML_CONTENT, cml_scalar_start, cml_scalar_end),
			GSF_XML_IN_NODE (CRYSTAL, CRYSTAL_SYMMETRY, -1, "symmetry", GSF_XML_NO_CONTENT, cml_symmetry_start, NULL),
				GSF_XML_IN_NODE (CRYSTAL_SYMMETRY, CRYSTAL_TRANSFORM, -1, "transform3", GSF_XML_CONTENT, NULL, cml_transform_end),
				GSF_XML_IN_NODE (CRYSTAL_SYMMETRY, CRYSTAL_MATRIX, -1, "matrix", GSF_XML_CONTENT, NULL, cml_transform_end),
	GSF_XML_IN_NODE_END
	};
	CMLReadState	*state = (CMLReadState *) xin->user_state;
	Object *obj = state->app->CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
	static GsfXMLInDoc *doc = NULL;
	if (NULL == doc)
		doc = gsf_xml_in_doc_new (mol_dtd, NULL);
	gsf_xml_in_push_state (xin, doc, state, (GsfXMLInExtDtor) NULL, attrs);
	state->doc->ObjectLoaded (obj);
}

////////////////////////////////////////////////////////////////////////////////
// Reading code
static GsfXMLInNode const cml_dtd[] = {
GSF_XML_IN_NODE (CML, CML, -1, "cml", GSF_XML_CONTENT, &cml_doc, NULL),
	GSF_XML_IN_NODE (CML, CML_SCALAR, -1, "scalar", GSF_XML_CONTENT, cml_scalar_start, cml_scalar_end),
	GSF_XML_IN_NODE (CML, MOLECULE, -1, "molecule", GSF_XML_CONTENT, cml_mol_start, cml_simple_end),
};

ContentType CMLLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	CMLReadState state;
	bool  success = false;

	state.doc = doc;
	state.app = doc->GetApplication ();
	state.context = io;
	state.cur.push (doc);
	state.type = ContentTypeMisc;

	doc->SetScale (100.);

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cml_dtd, NULL);
		success = gsf_xml_in_doc_parse (xml, in, &state);

		if (!success)
			go_io_warning (state.context,
				_("'%s' is corrupt!"),
				gsf_input_name (in));
		gsf_xml_in_doc_free (xml);
	}
	return success? state.type: ContentTypeUnknown;
}

////////////////////////////////////////////////////////////////////////////////
// Writing code

bool CMLLoader::WriteObject (GsfXMLOut *xml, Object const *object, GOIOContext *io, ContentType type)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CMLLoader *, GsfXMLOut *, Object const *, GOIOContext *, ContentType)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, xml, object, io, type);
	// if we don't save the object iself, try to save its children
	std::map <std::string, Object *>::const_iterator j;
	Object const *child = object->GetFirstChild (j);
	while (child) {
		if (!WriteObject (xml, child, io, type))
			return false;
		child = object->GetNextChild (j);
	}
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

bool CMLLoader::Write  (Object const *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, GOIOContext *io, ContentType type)
{
	if (NULL != out) {
		Document const *doc = dynamic_cast <Document const *> (obj);
		if (!doc)
			doc = obj->GetDocument ();
		GsfXMLOut *xml = gsf_xml_out_new (out);
		gsf_xml_out_start_element (xml, "cml");
		gsf_xml_out_add_cstr_unchecked (xml, "xmlns:cml", "http://www.xml-cml.org/schema");
		// FIXME: add other namespaces if needed
		if (doc) {
			const_cast <Document *> (doc)->SetScale (100);
			string title = doc->GetProperty (GCU_PROP_DOC_TITLE);
			if (title.length ())
				gsf_xml_out_add_cstr (xml, "title", title.c_str ());
			if (type == ContentTypeCrystal) {
				gsf_xml_out_start_element (xml, "molecule");
				gsf_xml_out_add_cstr (xml, "id", "mol0");
				gsf_xml_out_start_element (xml, "crystal");
				// exporting cell parameters using old cml style
				// FIXME: use cellParameter
				// a
				title = doc->GetProperty (GCU_PROP_CELL_A);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "a");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// b
				title = doc->GetProperty (GCU_PROP_CELL_B);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "b");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// c
				title = doc->GetProperty (GCU_PROP_CELL_C);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "c");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// alpha
				title = doc->GetProperty (GCU_PROP_CELL_ALPHA);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "alpha");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// beta
				title = doc->GetProperty (GCU_PROP_CELL_BETA);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "beta");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// gamma
				title = doc->GetProperty (GCU_PROP_CELL_GAMMA);
				gsf_xml_out_start_element (xml, "scalar");
				gsf_xml_out_add_cstr (xml, "title", "gamma");
				gsf_xml_out_add_cstr (xml, "units", "units:angstrom");
				gsf_xml_out_add_cstr_unchecked (xml, NULL, title.c_str ());
				gsf_xml_out_end_element (xml);
				// space group
				title = doc->GetProperty (GCU_PROP_SPACE_GROUP);
				gsf_xml_out_start_element (xml, "symmetry");
				gsf_xml_out_add_cstr (xml, "spaceGroup", title.c_str ());
				std::list <Transform3d*>::const_iterator t;
				SpaceGroup const *group = SpaceGroup::GetSpaceGroup (title);
				Transform3d const *tr = group->GetFirstTransform (t);
				while (tr) {
					gsf_xml_out_start_element (xml, "transform3");
					gsf_xml_out_add_cstr_unchecked (xml, NULL, tr->DescribeAsValues().c_str ());
					gsf_xml_out_end_element (xml);
					tr = group->GetNextTransform (t);
				}
				gsf_xml_out_end_element (xml);
				// close <crystal> node
				gsf_xml_out_end_element (xml);
				// start atoms array
				gsf_xml_out_start_element (xml, "atomArray");
			}
			if (doc == obj) {
				std::map <std::string, Object *>::const_iterator i;
				Object const *child = doc->GetFirstChild (i);
				while (child) {
					if (!WriteObject (xml, child, io, type)) {
						g_object_unref (xml);
						return false;
					}
					child = doc->GetNextChild (i);
				}
			} else if (!WriteObject (xml, obj, io, type)) {
				g_object_unref (xml);
				return false;
			}
		} else {
			doc = obj->GetDocument ();
			const_cast <Document *> (doc)->SetScale (100);
			WriteObject (xml, obj, io, type);
		}
		if (type == ContentTypeCrystal) {
			gsf_xml_out_end_element (xml);
			gsf_xml_out_end_element (xml);
		}
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
