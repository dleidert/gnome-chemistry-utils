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
#include <gcu/application.h>
#include <gcu/atom.h>
#include <gcu/bond.h>
#include <gcu/document.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
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
#include <cstring>

#include <iostream>

using namespace std;
using namespace gcu;

static map<string, unsigned> KnownProps;

typedef struct {
	unsigned index;
	string encoding;
	string name;
} CDXMLFont;

class CDXMLLoader: public gcu::Loader
{
public:
	CDXMLLoader ();
	virtual ~CDXMLLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Object *obj, GsfOutput *out, char const *mime_type, IOContext *io, ContentType type);

private:
	bool WriteObject (xmlDocPtr xml, xmlNodePtr node, Object *object, IOContext *io);
	static void AddIntProperty (xmlNodePtr node, char const *id, int value);
	static void AddStringProperty (xmlNodePtr node, char const *id, string &value);
	static bool WriteAtom (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, IOContext *s);
	static bool WriteBond (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, IOContext *s);
	static bool WriteMolecule (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, IOContext *s);

private:
	map <string, bool (*) (CDXMLLoader *, xmlDocPtr, xmlNodePtr, Object *, IOContext *)> m_WriteCallbacks;
	map <unsigned, GOColor> m_Colors;
	map <unsigned, CDXMLFont> m_Fonts;
	map <string, unsigned> m_SavedIds;
	int m_MaxId;
};

CDXMLLoader::CDXMLLoader ()
{
	AddMimeType ("chemical/x-cdxml");
	KnownProps["BondLength"] = GCU_PROP_THEME_BOND_LENGTH;
	KnownProps["Comment"] = GCU_PROP_DOC_COMMENT;
	KnownProps["CreationDate"] = GCU_PROP_DOC_CREATION_TIME;
	KnownProps["CreationUserName"] = GCU_PROP_DOC_CREATOR;
	KnownProps["ModificationDate"] = GCU_PROP_DOC_MODIFICATION_TIME;
	KnownProps["Name"] = GCU_PROP_DOC_TITLE;
	KnownProps["p"] = GCU_PROP_POS2D;
	KnownProps["Element"] = GCU_PROP_ATOM_Z;
	KnownProps["Charge"] = GCU_PROP_ATOM_CHARGE;
	KnownProps["id"] = GCU_PROP_ID;
	KnownProps["B"] = GCU_PROP_BOND_BEGIN;
	KnownProps["DISPLAY"] = GCU_PROP_BOND_TYPE;
	KnownProps["E"] = GCU_PROP_BOND_END;
	KnownProps["Order"] = GCU_PROP_BOND_ORDER;
	KnownProps["LabelJustification"] =GCU_PROP_TEXT_JUSTIFICATION;
	KnownProps["Justification"] =GCU_PROP_TEXT_JUSTIFICATION;
	KnownProps["LabelAlignment"] = GCU_PROP_TEXT_ALIGNMENT;
	// Add write callbacks
	m_WriteCallbacks["atom"] = WriteAtom;
	m_WriteCallbacks["bond"] = WriteBond;
	m_WriteCallbacks["molecule"] = WriteMolecule;
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
	Document *doc;
	IOContext *context;
	stack<Object*> cur;
	list<CDXMLProps> failed;
	map<unsigned, CDXMLFont> fonts;
	vector<string> colors;
	string markup;
	unsigned attributes;
	unsigned font;
	unsigned color;
	string size;
} CDXMLReadState;

static void
cdxml_simple_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->cur.top ()->Lock (false);
	state->cur.top ()->OnLoaded ();
	state->cur.pop ();
}

static void
cdxml_doc (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	map<string, unsigned>::iterator it;
	while (*attrs) {
		if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
			state->doc->SetProperty ((*it).second, (char const *) *attrs);}
		attrs++;
	}
	state->cur.push (state->doc);
}

static void
cdxml_fragment_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
}

static void
cdxml_fragment_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	static_cast <Molecule*> (state->cur.top ())->UpdateCycles ();
	state->cur.top ()->Lock (false);
	state->cur.top ()->OnLoaded ();
	state->cur.pop ();
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
		if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
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
cdxml_text_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("text", state->cur.top ());
	state->cur.push (obj);
	char *lowered;
	map<string, unsigned>::iterator it;
	while (*attrs)
		if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
			lowered = g_ascii_strdown ((char const *) *attrs++, -1);
			obj->SetProperty ((*it).second, lowered);
			g_free (lowered);
		}
	state->markup = "<text>";
}

static void
cdxml_text_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->markup += "</text>";
	state->cur.top ()->SetProperty (GCU_PROP_TEXT_MARKUP, state->markup.c_str ());
	state->markup.clear ();
	state->cur.pop ();
}

static void
cdxml_string_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->attributes = 0;
	while (*attrs) {
		if (!strcmp ((char const *) *attrs, "font")) {
			attrs++;
			state->font = atoi ((char const *) *attrs);
			state->markup += "<font name=\"";
			state->markup += state->fonts[state->font].name;
			state->markup += " ";
		} else if (!strcmp ((char const *) *attrs, "face"))  {
			attrs++;
			state->attributes |= atoi ((char const *) *attrs);
		} else if (!strcmp ((char const *) *attrs, "size"))  {
			attrs++;
			state->size = (char const *) *attrs;
		} else if (!strcmp ((char const *) *attrs, "color"))  {
			attrs++;
			state->attributes |= 0x100;
			state->color = atoi ((char const *) *attrs);
		} else
			attrs ++;
		attrs++;
	}
	state->markup += string (" ") + state->size + "\">";
	if (state->attributes & 0x100)
		state->markup += string ("<fore ") + state->colors[state->color] + ">";
	if (state->attributes & 1)
		state->markup += "<b>";
	if (state->attributes & 2)
		state->markup += "<i>";
	if (state->attributes & 4)
		state->markup += "<u>";
	if ((state->attributes & 0x60) != 0x60) {
		if (state->attributes & 0x20)
			state->markup += "<sub>";
		else if (state->attributes & 0x40)
			state->markup += "<sup>";
	}
		
	// TODO: parse attributes
}

static void
cdxml_string_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	bool opened = true;
	//TODO: add xin->content->str
	if ((state->attributes & 0x60) == 0x60) {
		// for now put all numbers as subscripts
		// FIXME: fix this kludgy code
		int cur = 0, size = strlen (xin->content->str);
		char *new_size = g_strdup_printf ("%g", strtod (state->size.c_str (), NULL) / 1.5);
		char *height = g_strdup_printf ("%g", strtod (state->size.c_str (), NULL) / 3.);
		while (cur < size) {
			while (cur < size && (xin->content->str[cur] < '0' || xin->content->str[cur] > '9'))
				state->markup += xin->content->str[cur++];
			if (cur < size) {
				if (state->attributes & 4)
					state->markup += "</u>";
				if (state->attributes & 2)
					state->markup += "</i>";
				if (state->attributes & 1)
					state->markup += "</b>";
				if (state->attributes & 0x100)
					state->markup += "</fore>";
				state->markup += string ("</font><font name=\"") + state->fonts[state->font].name + " " + new_size + "\">";
				if (state->attributes & 0x100)
					state->markup += string ("<fore ") + state->colors[state->color] + ">";
				state->markup += string ("<sub height=\"") + height + "\">";
				while (xin->content->str[cur] >= '0' && xin->content->str[cur] <= '9')
					state->markup += xin->content->str[cur++];
				state->markup += "</sub>";
				if (state->attributes & 0x100)
					state->markup += "</fore>";
				state->markup += string ("</font>");
				if (cur < size) {
					state->markup += string ("<font name=\"") + state->fonts[state->font].name + " " + state->size + "\">";
					if (state->attributes & 0x100)
						state->markup += string ("<fore ") + state->colors[state->color] + ">";
					if (state->attributes & 1)
						state->markup += "<b>";
					if (state->attributes & 2)
						state->markup += "<i>";
					if (state->attributes & 4)
						state->markup += "<u>";
				} else
					opened = false;
			}
		}
		g_free (new_size);
		g_free (height);
	} else {
		state->markup += xin->content->str;
		if (state->attributes & 0x20)
			state->markup += "</sub>";
		else if (state->attributes & 0x40)
			state->markup += "</sup>";
	}
	if (opened) {
		if (state->attributes & 4)
			state->markup += "</u>";
		if (state->attributes & 2)
			state->markup += "</i>";
		if (state->attributes & 1)
			state->markup += "</b>";
		if (state->attributes & 0x100)
			state->markup += "</fore>";
		state->markup += "</font>";
	}
	state->attributes = 0;
}

static void
fragment_done (G_GNUC_UNUSED GsfXMLIn *xin, CDXMLReadState *state)
{
	Object *atom = state->cur.top (), *child;
	state->cur.pop ();
	map <string, Object *>::iterator i;
	Molecule *mol = NULL, *mol1 = NULL;
	string buf;
	//TODO: retreive text and molecule and compare
	while ((child = atom->GetFirstChild (i))) {
		child->SetParent (NULL);
		if (child->GetType () == MoleculeType)
			mol = dynamic_cast <Molecule *> (child);
		else {
			buf = child->GetProperty (GCU_PROP_TEXT_TEXT);
			delete child;
		}
	}
	if (mol) {
		if (buf.length () > 0) {
			try {
				Formula form (buf, GCU_FORMULA_PARSE_RESIDUE);
				mol1 = Molecule::MoleculeFromFormula (state->doc, form);
				bool have_pseudo = false;
				Object *obj = mol->GetFirstChild (i);
				gcu::Atom *a = NULL;
				while (obj) {
					a = dynamic_cast <gcu::Atom *> (obj);
					if (a && ! a->GetZ ()) {
						have_pseudo = true;
						break;
					}
					obj = mol->GetNextChild (i);
				}
				if (!mol1 || !(*mol == *mol1)) {
					if (have_pseudo) {
						// try adding a new residue
						// first examine the first atom
						map <gcu::Atom*, gcu::Bond*>::iterator i;
						gcu::Bond *b = a->GetFirstBond (i);
						int residue_offset = 0;
						if (!b)
							goto fragment_error;
						gcu::Atom *a2 = b->GetAtom (a);
						if (!a2)
							goto fragment_error;
						list<FormulaElt *> const &elts = form.GetElements ();
						list<FormulaElt *>::const_iterator j = elts.begin ();
						FormulaAtom *fatom = dynamic_cast <FormulaAtom *> (*j);
						int valence;
						if (!fatom || fatom->elt != a2->GetZ ())
							goto fragment_add;
						valence = Element::GetElement (fatom->elt)->GetDefaultValence ();
						switch (valence) {
						case 2: {
							/* remove the first atom and replace it by a pseudo-atom, then add the residue
							this helps with things begining with an oxygen or a sulfur, but might be 
							not enough n other cases */
							double x, y;
							a2->GetCoords (&x, &y);
							a->SetCoords (x, y);
							a->RemoveBond (b);
							a2->RemoveBond (b);
							mol->Remove (b);
							delete b;
							if (a2->GetBondsNumber () > 1)
								goto fragment_error;
							b = a2->GetFirstBond (i);
							if (b->GetOrder () != 1)
								goto fragment_error;
							b->ReplaceAtom (a2, a);
							a->AddBond (b);
							mol->Remove (a2);
							delete a2;
							// now remove the atom from the new residue symbol
							residue_offset += fatom->end;
							break;
						}
						case 3:
							// we do not support that at the moment
							goto fragment_error;
							break;
						default:
							// we do not support that at the moment
							goto fragment_error;
						}
fragment_add:
						// Try create a new document, using the symbol as name
						// reparent the molecule to avoid a crash
						state->doc->AddChild (mol);
						state->doc->CreateResidue (buf.c_str () + residue_offset, buf.c_str () + residue_offset, mol);
						mol = NULL;
						goto fragment_success;
					}
fragment_error:
					g_warning (_("failed for %s\n"),buf.c_str ());
				}
			}
			catch (parse_error &error) {
				int start, length;
				puts (error.what (start, length));
			}
fragment_success:
			string pos = atom->GetProperty (GCU_PROP_POS2D);
			string id = atom->GetId ();
			mol = reinterpret_cast <Molecule *> (state->cur.top ());
			mol->Remove (atom);
			delete atom;
			atom = Object::CreateObject ("fragment", mol);
			atom->SetProperty (GCU_PROP_TEXT_TEXT, buf.c_str ());
			atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_ID, id.c_str ());
			atom->SetProperty (GCU_PROP_FRAGMENT_ATOM_START, "0");
			atom->SetProperty (GCU_PROP_POS2D, pos.c_str ());
			if (mol1) {
				mol1->SetParent (NULL);
				delete mol1;
			}
			mol = NULL;
		}
		if (mol)
			delete mol;
	}
}

static void
cdxml_node_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	static GsfXMLInNode const atom_dtd[] = {
	GSF_XML_IN_NODE (ATOM, ATOM, -1, "n", GSF_XML_CONTENT, NULL, NULL),
			GSF_XML_IN_NODE (ATOM, T, -1, "t", GSF_XML_CONTENT, cdxml_text_start, cdxml_text_end),
				GSF_XML_IN_NODE (T, S, -1, "s", GSF_XML_CONTENT, cdxml_string_start, cdxml_string_end),
			GSF_XML_IN_NODE (ATOM, FRAGMENT, -1, "fragment", GSF_XML_CONTENT, cdxml_fragment_start, cdxml_fragment_end),
				GSF_XML_IN_NODE (FRAGMENT, NODE, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT, BOND, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT, T1, -1, "t", GSF_XML_CONTENT, NULL, NULL),
					GSF_XML_IN_NODE (T1, S1, -1, "s", GSF_XML_CONTENT, cdxml_string_start, cdxml_string_end),
	GSF_XML_IN_NODE_END
	};
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("atom", state->cur.top ());
	obj->SetProperty (GCU_PROP_ATOM_Z, "6");
	map<string, unsigned>::iterator it;
	bool fragment = false;
	while (*attrs) {
		if ((it = KnownProps.find ((char const *) *attrs)) != KnownProps.end ()) {
			attrs++;
			obj->SetProperty ((*it).second, (char const *) *attrs);
		} else if (!strcmp ((char const *) *attrs++, "NodeType")) {
			if (!strcmp ((char const *) *attrs, "Fragment") ||
				!strcmp ((char const *) *attrs, "Nickname") ||
				!strcmp ((char const *) *attrs, "Unspecified") ||
				!strcmp ((char const *) *attrs, "GenericNickname"))
				fragment = true;
			else if (!strcmp ((char const *) *attrs, "ExternalConnectionPoint")) {
				// convert the atom to a pseudo atom.
				string pos = obj->GetProperty (GCU_PROP_POS2D);
				string id = obj->GetProperty (GCU_PROP_ID);
				Molecule *mol = dynamic_cast <Molecule *> (state->cur.top ());
				if (mol)
					mol->Remove (obj);
				delete obj;
				obj = Object::CreateObject ("pseudo-atom", state->cur.top ());
				if (id.length ())
					obj->SetProperty (GCU_PROP_ID, id.c_str ());
				obj->SetProperty (GCU_PROP_POS2D, pos.c_str ());
			}
			attrs++;
		}
		attrs++;
	}
	state->cur.push (obj);
	if (fragment) {
		static GsfXMLInDoc *doc = NULL;
		if (NULL == doc)
			doc = gsf_xml_in_doc_new (atom_dtd, NULL);
		state->cur.push (obj); // push it twice in that case
		gsf_xml_in_push_state (xin, doc, state, (GsfXMLInExtDtor) fragment_done, attrs);
	}
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
	state->colors.push_back (string ("red=\"") + red + "\" green=\"" + green + "\" blue=\"" + blue + "\"");
}

static void
cdxml_group_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = Object::CreateObject ("group", state->cur.top ());
	obj->Lock ();
	state->cur.push (obj);
}

static void
cdxml_graphic_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	guint32 Id = 0;
	guint16 type = 0xffff, arrow_type = 0xffff;
	double x0, y0, x1, y1;
	while (*attrs) {
		if (!strcmp ((char const *) *attrs, "id"))
			Id = atoi ((char const *) attrs[1]);
		else if (!strcmp ((char const *) *attrs, "BoundingBox"))
			sscanf ((char const *) attrs[1], "%lg %lg %lg %lg", &x1, &y1, &x0, &y0);
		else if (!strcmp ((char const *) *attrs, "GraphicType")) {
			if (!strcmp ((char const *) attrs[1], "Line"))
				type = 1;
		} else if (!strcmp ((char const *) *attrs, "ArrowType")) {
			if (!strcmp ((char const *) attrs[1], "FullHead") || !strcmp ((char const *) attrs[1], "HalfHead"))
				arrow_type = 2;
			else if (!strcmp ((char const *) attrs[1], "Resonance"))
				arrow_type = 4;
			else if (!strcmp ((char const *) attrs[1], "Equilibrium"))
				arrow_type = 8;
			else if (!strcmp ((char const *) attrs[1], "Hollow"))
				arrow_type = 16;
			else if (!strcmp ((char const *) attrs[1], "RetroSynthetic"))
				arrow_type = 32;
		}
		attrs+=2;
	}
	if (type == 1) {
		char *buf = NULL;
		Object *obj = NULL;
		switch (arrow_type) {
		case 1:
		case 2:
			obj = Object::CreateObject ("reaction-arrow", state->cur.top ());
			buf = g_strdup_printf ("ra%d", Id);
			break;
		case 4:
			obj = Object::CreateObject ("mesomery-arrow", state->cur.top ());
			buf = g_strdup_printf ("ma%d", Id);
			break;
		case 8:
			obj = Object::CreateObject ("reaction-arrow", state->cur.top ());
			buf = g_strdup_printf ("ra%d", Id);
			obj->SetProperty (GCU_PROP_REACTION_ARROW_TYPE, "double");
			break;
		case 32:
			obj = Object::CreateObject ("retrosynthesis-arrow", state->cur.top ());
			buf = g_strdup_printf ("rsa%d", Id);
			break;
		default:
			break;
		}
		if (obj) {
			obj->SetId (buf);
			g_free (buf);
			buf = g_strdup_printf ("%g %g %g %g", x0, y0, x1, y1);
			obj->SetProperty (GCU_PROP_ARROW_COORDS, buf);
			g_free (buf);
		}
	}
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
		GSF_XML_IN_NODE (PAGE, T, -1, "t", GSF_XML_NO_CONTENT, cdxml_text_start, cdxml_simple_end),
			GSF_XML_IN_NODE (T, S, -1, "s", GSF_XML_CONTENT, cdxml_string_start, cdxml_string_end),
		GSF_XML_IN_NODE (PAGE, FRAGMENT, -1, "fragment", GSF_XML_CONTENT, &cdxml_fragment_start, &cdxml_fragment_end),
			GSF_XML_IN_NODE (FRAGMENT, NODE, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
			GSF_XML_IN_NODE (FRAGMENT, BOND, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
			GSF_XML_IN_NODE (FRAGMENT, T1, -1, "t", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GROUP, -1, "group", GSF_XML_CONTENT, cdxml_group_start, cdxml_simple_end),
			GSF_XML_IN_NODE (GROUP, FRAGMENT1, -1, "fragment", GSF_XML_CONTENT, cdxml_fragment_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, NODE1, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, BOND1, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
				GSF_XML_IN_NODE (FRAGMENT1, T11, -1, "t", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GRAPHIC, -1, "graphic", GSF_XML_CONTENT, cdxml_graphic_start, NULL),
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

ContentType CDXMLLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, IOContext *io)
{
	CDXMLReadState state;

	state.doc = doc;
	state.context = io;
	ContentType success = ContentTypeUnknown;
	state.colors.push_back ("red=\"1\" green=\"1\" blue=\"1\""); // white
	state.colors.push_back ("red=\"0\" green=\"0\" blue=\"0\""); // black
	state.font = 0;
	state.color = 0;

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cdxml_dtd, NULL);
		if (gsf_xml_in_doc_parse (xml, in, &state))
			success = ContentType2D;

		if (success == ContentTypeUnknown)
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
					success = ContentTypeUnknown;
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

static bool start = true;
static int cb_xml_to_vfs (GsfOutput *output, const guint8* buf, int nb)
{
	if (start) {
		char const *end = strchr (reinterpret_cast <char const *> (buf), '\n');
		gsf_output_write (output, 40, reinterpret_cast <guint8 const *> ("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"));
		gsf_output_write (output, 70, reinterpret_cast <guint8 const *> ("<!DOCTYPE CDXML SYSTEM \"http://www.cambridgesoft.com/xml/cdxml.dtd\" >\n"));
		start = false;
		return gsf_output_write (output, strlen (end) - 1, reinterpret_cast <guint8 const *> (end + 1))? nb: 0;
	} else
		return gsf_output_write (output, nb, buf)? nb: 0;
}

bool CDXMLLoader::WriteAtom (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, G_GNUC_UNUSED IOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("n"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	string prop = obj->GetProperty (GCU_PROP_POS2D);
	AddStringProperty (node, "p", prop);
	prop = obj->GetProperty (GCU_PROP_ATOM_Z);
	if (prop != "6")
		AddStringProperty (node, "Element", prop);
	return true;
}

bool CDXMLLoader::WriteBond (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, G_GNUC_UNUSED IOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("b"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	string prop = obj->GetProperty (GCU_PROP_BOND_BEGIN);
	AddIntProperty (node, "B", loader->m_SavedIds[prop]);
	prop = obj->GetProperty (GCU_PROP_BOND_END);
	AddIntProperty (node, "E", loader->m_SavedIds[prop]);
	prop = obj->GetProperty (GCU_PROP_BOND_ORDER);
	if (prop == "3")
		prop = "4";
	else if (prop != "2")
		prop.clear ();
	if (prop.length ())
		AddStringProperty (node, "Order", prop);
	prop = obj->GetProperty (GCU_PROP_BOND_TYPE);
	if (prop == "wedge")
		prop = "WedgeBegin";
	else if (prop == "hash")
		prop = "WedgedHashBegin";
	else if (prop == "squiggle")
		prop = "Wavy";
	else
		prop.clear ();
	if (prop.length ())
		AddStringProperty (node, "Display", prop);
	return true;
}

bool CDXMLLoader::WriteMolecule (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object *obj, IOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("fragment"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	// save atoms
	std::map <std::string, Object *>::iterator i;
	Object *child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == AtomType && !loader->WriteObject (xml, node, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	// save fragments
	child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == FragmentType && !loader->WriteObject (xml, node, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	// save bonds
	child = obj->GetFirstChild (i);
	while (child) {
		if (child->GetType () == BondType && !loader->WriteObject (xml, node, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	return true;
}

bool CDXMLLoader::WriteObject (xmlDocPtr xml, xmlNodePtr node, Object *object, IOContext *io)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CDXMLLoader *, xmlDocPtr, xmlNodePtr, Object *, IOContext *)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, xml, node, object, io);
	// if we don't save the object iself, try to save its children
	std::map <std::string, Object *>::iterator j;
	Object *child = object->GetFirstChild (j);
	while (child) {
		if (!WriteObject (xml, node, child, io))
			return false;
		child = object->GetNextChild (j);
	}
	return true; /* loosing data is not considered an error, it is just a missing feature
					either in this code or in the cml schema */
}

void CDXMLLoader::AddIntProperty (xmlNodePtr node, char const *id, int value)
{
	char *buf = g_strdup_printf("%d", value);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (id), reinterpret_cast <xmlChar const *> (buf));
	g_free (buf);
}

void CDXMLLoader::AddStringProperty (xmlNodePtr node, char const *id, string &value)
{
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (id), reinterpret_cast <xmlChar const *> (value.c_str ()));
}

bool CDXMLLoader::Write  (Object *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED IOContext *io, G_GNUC_UNUSED ContentType type)
{
	map<string, CDXMLFont> fonts;
	Document *doc = dynamic_cast <Document *> (obj);
	xmlNodePtr colors, fonttable, page;
	if (!doc || !out)
		return false;

	m_MaxId = 1;

	// Init default colors
	m_Colors[2] = RGBA_WHITE;
	m_Colors[3] = RGBA_BLACK;
	m_Colors[4] = RGBA_RED;
	m_Colors[5] = RGBA_YELLOW;
	m_Colors[6] = RGBA_GREEN;
	m_Colors[7] = RGBA_CYAN;
	m_Colors[8] = RGBA_BLUE;
	m_Colors[9] = RGBA_VIOLET;

	// Init fonts, we always use Unknown as the charset, hoping it is not an issue
	m_Fonts[3] = (CDXMLFont) {3, string ("Unknown"), string ("Arial")};
	m_Fonts[4] = (CDXMLFont) {4, string ("Unknown"), string ("Times New Roman")};

	/* we can't use sax, because we need colors and fonts */
	xmlDocPtr xml = xmlNewDoc (reinterpret_cast <xmlChar const *> ("1.0"));
	xmlDocSetRootElement (xml,  xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("CDXML"), NULL));
	std::string app = doc->GetApp ()->GetName () + " "VERSION;
	xmlNewProp (xml->children, reinterpret_cast <xmlChar const *> ("CreationProgram"),
	            reinterpret_cast <xmlChar const *> (app.c_str ()));
	// determine the bond length and scale the document appropriately
	string prop = doc->GetProperty (GCU_PROP_THEME_BOND_LENGTH);
	double scale = strtod (prop.c_str (), NULL);
	doc->SetScale (scale / 30.);
	xmlNewProp (xml->children, reinterpret_cast <xmlChar const *> ("BondLength"),
	            reinterpret_cast <xmlChar const *> ("30"));
	// add color table
	colors = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("colortable"), NULL);
	xmlAddChild (xml->children, colors);
	// add font table
	fonttable = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("fonttable"), NULL);
	xmlAddChild (xml->children, fonttable);
	// start page
	page = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("page"), NULL);
	xmlAddChild (xml->children, page);
	// build tree from children
	std::map <std::string, Object *>::iterator i;
	Object *child = doc->GetFirstChild (i);
	while (child) {
		if (!WriteObject (xml, page, child, io)) {
			xmlFreeDoc (xml);
			m_Colors.clear ();
			m_Fonts.clear ();
			m_SavedIds.clear ();
			return false;
		}
		child = doc->GetNextChild (i);
	}
	// add colors to color table
	map <unsigned, GOColor>::iterator color, end_color = m_Colors.end ();
	for (color = m_Colors.begin (); color != end_color; color++) {
		xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("color"), NULL);
		xmlAddChild (colors, node);
		char *buf = g_strdup_printf ("%g", DOUBLE_RGBA_R ((*color).second));
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("r"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_G ((*color).second));
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("g"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_B ((*color).second));
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("b"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
	}
	// write fonts
	map <unsigned, CDXMLFont>::iterator font, end_font = m_Fonts.end ();
	for (font = m_Fonts.begin (); font != end_font; font++) {
		xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("font"), NULL);
		xmlAddChild (fonttable, node);
		AddIntProperty (node, "id", (*font).second.index);
		AddStringProperty (node, "charset", (*font).second.encoding);
		AddStringProperty (node, "name", (*font).second.name);
	}
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	xmlOutputBufferPtr buf = xmlAllocOutputBuffer (NULL);
	buf->context = out;
	buf->closecallback = NULL;
	buf->writecallback = (xmlOutputWriteCallback) cb_xml_to_vfs;
	start = true;
	xmlSaveFormatFileTo (buf, xml, NULL, true);
	xmlFreeDoc (xml);
	m_Colors.clear ();
	m_Fonts.clear ();
	m_SavedIds.clear ();
	return true;
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
