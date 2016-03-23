// -*- C++ -*-

/*
 * CDXML files loader plugin
 * cdxml.cc
 *
 * Copyright (C) 2007-2016 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcu/application.h>
#include <gcu/atom.h>
#include <gcu/bond.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/loader.h>
#include <gcu/molecule.h>
#include <gcu/objprops.h>
#include <gcu/xml-utils.h>

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
#include <sstream>

using namespace std;
using namespace gcu;

static map<string, unsigned> KnownProps;

typedef struct {
	unsigned index;
	string encoding;
	string name;
} CDXMLFont;

typedef struct {
	xmlDocPtr xml;
	xmlNodePtr node;
	xmlNodePtr parent;
	GOIOContext *s;
	bool italic;
	bool bold;
	bool underline;
	unsigned font;
	double size;
	int position;
	unsigned color;
} WriteTextState;

class CDXMLLoader: public gcu::Loader
{
public:
	CDXMLLoader ();
	virtual ~CDXMLLoader ();

	ContentType Read (Document *doc, GsfInput *in, char const *mime_type, GOIOContext *io);
	bool Write (Object const *obj, GsfOutput *out, char const *mime_type, GOIOContext *io, ContentType type);

private:
	bool WriteObject (xmlDocPtr xml, xmlNodePtr node, Object const *object, GOIOContext *io);
	static void AddIntProperty (xmlNodePtr node, char const *id, int value);
	static void AddFloatProperty (xmlNodePtr node, char const *id, double value);
	static void AddStringProperty (xmlNodePtr node, char const *id, string const &value);
	static bool WriteArrow (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteAtom (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteFragment (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteBond (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteMesomery(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteMolecule (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteReaction(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteReactionStep(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteRetrosynthesis(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteText(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s);
	static bool WriteNode (CDXMLLoader *loader, xmlNodePtr node, WriteTextState *state);

private:
	map <string, bool (*) (CDXMLLoader *, xmlDocPtr, xmlNodePtr, Object const *, GOIOContext *)> m_WriteCallbacks;
	map <unsigned, GOColor> m_Colors;
	map <unsigned, CDXMLFont> m_Fonts;
	map <string, unsigned> m_SavedIds;
	int m_MaxId;
	unsigned m_Z;
	int m_LabelFont, m_Font;
	unsigned m_LabelFontFace, m_LabelFontColor;
	double m_FontSize, m_LabelFontSize, m_Scale, m_Zoom, m_CHeight;
};

CDXMLLoader::CDXMLLoader ()
{
	AddMimeType ("chemical/x-cdxml");
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
	KnownProps["Display"] = GCU_PROP_BOND_TYPE;
	KnownProps["E"] = GCU_PROP_BOND_END;
	KnownProps["Order"] = GCU_PROP_BOND_ORDER;
	KnownProps["DoublePosition"] = GCU_PROP_BOND_DOUBLE_POSITION;
	KnownProps["LabelJustification"] =GCU_PROP_TEXT_JUSTIFICATION;
	KnownProps["CaptionJustification"] =GCU_PROP_TEXT_ALIGNMENT;
	KnownProps["LabelAlignment"] = GCU_PROP_TEXT_ALIGNMENT;
	KnownProps["Justification"] =GCU_PROP_TEXT_JUSTIFICATION;
	// Add write callbacks
	m_WriteCallbacks["atom"] = WriteAtom;
	m_WriteCallbacks["fragment"] = WriteFragment;
	m_WriteCallbacks["bond"] = WriteBond;
	m_WriteCallbacks["molecule"] = WriteMolecule;
	m_WriteCallbacks["text"] = WriteText;
	m_WriteCallbacks["reaction"] = WriteReaction;
	m_WriteCallbacks["reaction-arrow"] = WriteArrow;
	m_WriteCallbacks["mesomery"] = WriteMesomery;
	m_WriteCallbacks["mesomery-arrow"] = WriteArrow;
	m_WriteCallbacks["retrosynthesis-arrow"] = WriteArrow;
	m_WriteCallbacks["retrosynthesis"] = WriteRetrosynthesis;
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
	std::list < unsigned > Arrows, Reagents, Products, ObjectsAbove, ObjectsBelow;
} StepData;

typedef struct {
	unsigned Id;
	std::list < StepData > Steps;
} SchemeData;

typedef struct {
	Document *doc;
	Application *app;
	gcp::Theme *theme;
	ostringstream themedesc;
	GOIOContext *context;
	stack<Object*> cur;
	list<CDXMLProps> failed;
	map<unsigned, CDXMLFont> fonts;
	std::map <unsigned, std::string> loaded_ids;
	vector<string> colors;
	string markup;
	unsigned attributes;
	unsigned font;
	unsigned color;
	string size;
	int line_height;
	unsigned captionFont, labelFont, textAlign;
	double CHeight, padding;
	SchemeData scheme;
	std::list < SchemeData > schemes;
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
	double bond_dist_ratio = -1., bond_length = 66.24, x;
	state->themedesc << "<?xml version=\"1.0\"?>" << std::endl << "<theme name=\"ChemDraw\"";
	state->doc->SetProperty (GCU_PROP_DOC_CREATOR, ""); // Chemdraw does not use it for now.
	if (attrs)
		while (*attrs) {
			std::string key = reinterpret_cast < char const * > (*attrs);
			if ((it = KnownProps.find (key)) != KnownProps.end ())
				state->doc->SetProperty ((*it).second, reinterpret_cast < char const * > (*(attrs + 1)));
			else if (key == "BondLength") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> bond_length;
				bond_length *= 8.;
				state->themedesc << " bond-length=\"" << bond_length << "\" zoom-factor=\"6\"";
				state->doc->SetScale (8.);
			} else if (key == "BondSpacing") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> bond_dist_ratio;
				bond_dist_ratio /= 100.;
			} else if (key == "LineWidth") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				x *= 4. / 3.;
				state->themedesc << " bond-width=\"" << x << "\" arrow-width=\"" << x << "\" hash-width=\"" << x << "\"";
			} else if (key == "BoldWidth") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				state->themedesc << " stereo-bond-width=\"" << x * 4. / 3. << "\"";
			} else if (key == "HashSpacing") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				state->themedesc << " hash-dist=\"" << x * 4. / 3. << "\"";
			} else if (key == "ChainAngle")
				state->themedesc << " bond-angle=\"" << reinterpret_cast < char const * > (*(attrs + 1)) << "\"";
			else if (key == "MarginWidth") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				x *= 4. / 3.;
				state->padding = x * 2.;
				state->themedesc << " padding=\"" << x << "\" arrow-padding=\"" << x << "\" object-padding=\"" << x << "\" sign-padding=\"" << x << "\"";
			} else if (key == "CaptionFont") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> state->captionFont;
			} else if (key == "CaptionSize") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				int size = x * PANGO_SCALE;
				state->themedesc << " text-font-size=\"" << size << "\"";
			} else if (key == "CaptionFace") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				int face;
				input >> face;
				switch (face & 3) { // we do not support anything else
				default:
				case 0:
					state->themedesc << " text-font-style=\"normal\" text-font-weight=\"normal\"";
					break;
				case 1:
					state->themedesc << " text-font-style=\"normal\" text-font-weight=\"bold\"";
					break;
				case 2:
					state->themedesc << " text-font-style=\"italic\" text-font-weight=\"normal\"";
					break;
				case 3:
					state->themedesc << " text-font-style=\"italic\" text-font-weight=\"bold\"";
					break;
				}
			} else if (key == "LabelFont") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> state->labelFont;
			} else if (key == "LabelSize") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> x;
				int size = x * PANGO_SCALE;
				state->themedesc << " font-size=\"" << size << "\"";
			} else if (key == "LabelFace") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				int face;
				input >> face;
				switch (face & 3) { // we do not support anything else
				default:
				case 0:
					state->themedesc << " font-style=\"normal\" font-weight=\"normal\"";
					break;
				case 1:
					state->themedesc << " font-style=\"normal\" font-weight=\"bold\"";
					break;
				case 2:
					state->themedesc << " font-style=\"italic\" font-weight=\"normal\"";
					break;
				case 3:
					state->themedesc << " font-style=\"italic\" font-weight=\"bold\"";
					break;
				}
			} else if (key == "CaptionJustification") {
				std::istringstream input (reinterpret_cast < char const * > (*(attrs + 1)));
				input >> state->textAlign;
			}
			attrs += 2;
		}
	if (bond_dist_ratio > 0.)
		state->themedesc << " bond-dist=\"" << bond_length * bond_dist_ratio / 6. << "\"";
	state->cur.push (state->doc);
}

static void
cdxml_page_start (GsfXMLIn *xin, xmlChar const **)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	// we need to set the theme when starting the first page
	if (state->theme != NULL)
		return;
	state->themedesc << "/>";
	gcp::Document *cpDoc = dynamic_cast <gcp::Document *> (state->doc);
	if (cpDoc != NULL) {
		xmlDocPtr xml = xmlParseMemory (state->themedesc.str().c_str(), state->themedesc.str().length());
		state->theme = new gcp::Theme (NULL);
		state->theme->Load (xml->children);
		xmlFreeDoc (xml);
		gcp::Theme *LocalTheme = gcp::TheThemeManager.GetTheme (state->theme->GetName ().c_str ());
		if (LocalTheme && *LocalTheme == *(state->theme)) {
			cpDoc->SetTheme (LocalTheme);
			delete state->theme;
			state->theme = LocalTheme;  // don't point to an invalid object
		} else {
			gcp::TheThemeManager.AddFileTheme (state->theme, state->doc->GetTitle ().c_str ());
			cpDoc->SetTheme (state->theme);
		}
		state->CHeight = cpDoc->GetView ()->GetCHeight (); // FIXME, we probably miss a factor, may be 6.
	}
}

static void
cdxml_fragment_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	Object *obj = state->app->CreateObject ("molecule", state->cur.top ());
	state->cur.push (obj);
	state->doc->ObjectLoaded (obj);
	if (attrs)
		while (*attrs) {
			if (!strcmp (reinterpret_cast < char const * > (*attrs), "id")) {
				unsigned id = atoi (reinterpret_cast < char const * > (attrs[1]));
				state->loaded_ids[id] = obj->GetId ();
			}
			attrs += 2;
		}
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
	Object *obj = state->app->CreateObject ("bond", state->cur.top ());
	obj->SetProperty (GCU_PROP_BOND_ORDER, "1");
	map<string, unsigned>::iterator it;
	if (attrs)
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
				} else if ((*it).second == GCU_PROP_BOND_DOUBLE_POSITION) {
					if (!strcmp (reinterpret_cast < char const * > (*attrs), "Center"))
					    obj->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "center");
					else if (!strcmp (reinterpret_cast < char const * > (*attrs), "Right"))
					    obj->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "right");
					else if (!strcmp (reinterpret_cast < char const * > (*attrs), "Left"))
					    obj->SetProperty (GCU_PROP_BOND_DOUBLE_POSITION, "left");
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
	state->doc->ObjectLoaded (obj);
}

static void
cdxml_text_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	gcu::Object *obj = state->app->CreateObject ("text", /*(state->cur.top ()->GetType () == gcu::DocumentType)? */state->cur.top ()/*: NULL*/);
	state->cur.push (obj);
	state->doc->ObjectLoaded (obj);
	char *lowered;
	state->line_height = 1;
	map<string, unsigned>::iterator it;
	if (attrs)
		while (*attrs)
			if (!strcmp (reinterpret_cast < char const * > (*attrs), "id")) {
				unsigned id = atoi (reinterpret_cast < char const * > (attrs[1]));
				state->loaded_ids[id] = obj->GetId ();
				attrs += 2;
			} else if (!strcmp (reinterpret_cast < char const * > (*attrs), "p")) {
				std::istringstream in (reinterpret_cast < char const * > (attrs[1]));
				double x, y;
				in >> x >> y;
				y -= state->CHeight;
				std::ostringstream out;
				out << x << " " << y;
				obj->SetProperty (GCU_PROP_POS2D, out.str ().c_str ());
				attrs += 2;
			} else if (!strcmp (reinterpret_cast < char const * > (*attrs), "LineHeight") ||
			           !strcmp (reinterpret_cast < char const * > (*attrs), "CaptionLineHeight")) {
				std::string val (reinterpret_cast < char const * > (attrs[1]));
				if (val == "auto")
					obj->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "false");
				else if (val == "variable")
					obj->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "false");
				else {
					std::istringstream in (val);
					in >> state->line_height;
				}
				
				attrs += 2;
			} else if ((it = KnownProps.find ((char const *) *attrs++)) != KnownProps.end ()) {
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
	if (state->cur.top ()->GetParent () == NULL)
		delete state->cur.top ();
	else if (state->line_height > 1) {
		state->cur.top ()->SetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT, "false");
		std::istringstream in (state->cur.top ()->GetProperty (GCU_PROP_TEXT_MAX_LINE_HEIGHT));
		double lh;
		in >> lh;
		std::ostringstream out;
		out << state->line_height - lh;
		state->cur.top ()->SetProperty (GCU_PROP_TEXT_INTERLINE, out.str ().c_str ());
	}

	state->cur.pop ();
}

static void
cdxml_string_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->attributes = 0;
	if (attrs)
		while (*attrs) {
			if (!strcmp ((char const *) *attrs, "font")) {
				attrs++;
				state->font = atoi ((char const *) *attrs);
				state->markup += "<font name=\"";
				state->markup += state->fonts[state->font].name;
				state->markup += ",";
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
	state->markup += state->size + "\">";
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
	if ((state->attributes & 0x60) == 0x60) {
		// for now put all numbers as subscripts
		// FIXME: fix this kludgy code
		int cur = 0, size = strlen (xin->content->str);
		char new_size[G_ASCII_DTOSTR_BUF_SIZE], height[G_ASCII_DTOSTR_BUF_SIZE];
		g_ascii_dtostr (new_size, G_ASCII_DTOSTR_BUF_SIZE, g_ascii_strtod (state->size.c_str (), NULL) / 1.5);
		g_ascii_dtostr (height, G_ASCII_DTOSTR_BUF_SIZE, g_ascii_strtod (state->size.c_str (), NULL) / 3.);
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
						map < gcu::Bondable *, gcu::Bond * >::iterator i;
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
			atom = state->app->CreateObject ("fragment", mol);
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
	} else goto fragment_success;
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
	Object *obj = state->app->CreateObject ("atom", state->cur.top ());
	obj->SetProperty (GCU_PROP_ATOM_Z, "6");
	state->doc->ObjectLoaded (obj);
	map<string, unsigned>::iterator it;
	bool fragment = false;
	if (attrs)
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
					obj = state->app->CreateObject ("pseudo-atom", state->cur.top ());
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
		state->doc->ObjectLoaded (obj);
		gsf_xml_in_push_state (xin, doc, state, (GsfXMLInExtDtor) fragment_done, attrs);
	}
}

static void
cdxml_font_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	CDXMLFont font;
	font.index = 0;
	if (attrs)
		while (*attrs) {
			if (!strcmp ((char const *) *attrs, "id"))
				font.index = atoi ((char const *) *(attrs + 1));
			else if (!strcmp ((char const *) *attrs, "charset"))
				font.encoding = (char const *) *(attrs + 1);
			else if (!strcmp ((char const *) *attrs, "name"))
				font.name = (char const *) *(attrs + 1);
			attrs += 2;
		}
	if (state->labelFont == font.index)
		state->themedesc << " font-family=\"" << font.name << "\"";
	if (state->captionFont == font.index)
		state->themedesc << " text-font-family=\"" << font.name << "\"";
	state->fonts[font.index] = font;
}

static void
cdxml_color (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	string red, green, blue;
	if (attrs)
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
	Object *obj = state->app->CreateObject ("group", state->cur.top ());
	obj->Lock ();
	state->cur.push (obj);
	state->doc->ObjectLoaded (obj);
}

static void
cdxml_graphic_start (GsfXMLIn *xin, xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	guint32 Id = 0;
	guint16 type = 0xffff, arrow_type = 0xffff;
	double x0, y0, x1, y1;
	if (attrs)
		while (*attrs) {
			if (!strcmp ((char const *) *attrs, "id"))
				Id = atoi ((char const *) attrs[1]);
			else if (!strcmp ((char const *) *attrs, "BoundingBox")) {
				istringstream str (reinterpret_cast <char const *> (attrs[1]));
				str >> x1 >> y1 >> x0 >> y0;
			} else if (!strcmp ((char const *) *attrs, "GraphicType")) {
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
		Object *obj = NULL;
		ostringstream str;
		switch (arrow_type) {
		case 1:
		case 2:
			obj = state->app->CreateObject ("reaction-arrow", state->cur.top ());
			str << "ra" << Id;
			break;
		case 4:
			obj = state->app->CreateObject ("mesomery-arrow", state->cur.top ());
			str << "ma" << Id;
			break;
		case 8:
			obj = state->app->CreateObject ("reaction-arrow", state->cur.top ());
			str << "ra" << Id;
			obj->SetProperty (GCU_PROP_REACTION_ARROW_TYPE, "double");
			break;
		case 32:
			obj = state->app->CreateObject ("retrosynthesis-arrow", state->cur.top ());
			str << "rsa" << Id;
			break;
		default:
			break;
		}
		if (obj) {
			obj->SetId (str.str ().c_str ());
			state->loaded_ids[Id] = str.str ();
			ostringstream str_;
			str_ << x0 << " " << y0 << " " << x1 << " " << y1;
			obj->SetProperty (GCU_PROP_ARROW_COORDS, str_.str ().c_str ());
			state->doc->ObjectLoaded (obj);
		}
	}
}

static void
cdxml_step_start (GsfXMLIn *xin, G_GNUC_UNUSED xmlChar const **attrs)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	StepData data;
	if (attrs)
		while (*attrs) {
			std::string key = reinterpret_cast < char const * > (*attrs);
			std::istringstream values (reinterpret_cast < char const * > (attrs[1]));
			attrs +=2;
			std::list < unsigned > *target;
			if (key == "ReactionStepReactants")
				target = &data.Reagents;
			else if (key == "ReactionStepProducts")
				target = &data.Products;
			else if (key == "ReactionStepArrows")
				target = &data.Arrows;
			else if (key == "ReactionStepObjectsAboveArrow")
				target = &data.ObjectsAbove;
			else if (key == "ReactionStepObjectsBelowArrow")
				target = &data.ObjectsBelow;
			else
				continue;
			while (!values.eof ()) {
				unsigned id;
				values >> id;
				target->push_back (id);
			}
		}
	state->scheme.Steps.push_back (data);
}

static void
cdxml_scheme_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	CDXMLReadState	*state = (CDXMLReadState *) xin->user_state;
	state->schemes.push_back (state->scheme);
	state->scheme.Steps.clear ();
}

static void build_scheme (CDXMLReadState &state, SchemeData &scheme)
{
	gcu::Document *doc = state.doc;
	std::list < StepData >::iterator i, iend = scheme.Steps.end ();
	std::list < unsigned >::iterator j, jend;
	int IsReaction = 0, IsMesomery = 0, IsRetrosynthesis = 0;
	bool HasMesomeryArrows = false;
	gcu::Object *parent, *arrow, *obj, *step, *reactant;
	for (i = scheme.Steps.begin (); i != iend; i++) {
		if ((*i).Arrows.size () != 1)
			return; // unsupported feature, don't load the scheme
		obj = doc->GetChild ((state.loaded_ids[*((*i).Arrows.begin())]).c_str ());
		if (obj == NULL)
			return;
		std::string klass = gcu::Object::GetTypeName (obj->GetType ());
		if (klass == "retrosynthesis-arrow") {
			if (IsRetrosynthesis == -1)
				return;
			IsRetrosynthesis = 1;
			IsReaction = IsMesomery = -1;
		} else if (klass ==  "mesomery-arrow") {
			if (IsMesomery == -1)
				return;
			IsRetrosynthesis = -1;
			if (IsMesomery == 0 && IsReaction == 0)
				IsMesomery = 1;
			HasMesomeryArrows = true;
		} else if (klass ==  "reaction-arrow") {
			if (IsReaction == -1 || IsMesomery == -1)
				return;
			IsReaction = 1;
			IsRetrosynthesis = -1;
			IsMesomery = 0;
		} else
			return;
	}
	if (IsRetrosynthesis == 1) {
	} else if (IsMesomery == 1) {
		gcu::Object *mesomery = doc->CreateObject ("mesomery", doc);
		ostringstream str;
		str << "msy" << scheme.Id;
		mesomery->SetId (str.str ().c_str ());
		state.loaded_ids[scheme.Id] = mesomery->GetId ();
		// now, add the objects to the reaction
		for (i = scheme.Steps.begin (); i != iend; i++) {
			if ((*i).Reagents.size () != 1 || (*i).Products.size () != 1) {
				delete mesomery;
				return;
			}
			// first the arrow
			arrow = doc->GetChild ((state.loaded_ids[*((*i).Arrows.begin())]).c_str ());
			obj = doc->GetDescendant (state.loaded_ids[*(*i).Reagents.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("mesomer", mesomery);
			else if (parent->GetParent () != mesomery) {
				delete mesomery;
				return;
			}
			parent->SetProperty (GCU_PROP_MESOMER, obj->GetId ());
			arrow->SetProperty (GCU_PROP_ARROW_START_ID, parent->GetId ());
			obj = doc->GetDescendant (state.loaded_ids[*(*i).Products.begin ()].c_str ());
			parent = obj->GetParent ();
			if (parent == doc)
				parent = doc->CreateObject ("mesomer", mesomery);
			else if (parent->GetParent () != mesomery) {
				delete mesomery;
				return;
			}
			parent->AddChild (obj);
			arrow->SetProperty (GCU_PROP_ARROW_END_ID, parent->GetId ());
			mesomery->AddChild (arrow);
		}
		// Ignore objects over and under the arrows for now
	} else if (IsReaction ==1) {
		if (HasMesomeryArrows) {
			// build mesomeries inside reactions
			// FIXME
		}
		gcu::Object *reaction = doc->CreateObject ("reaction", doc);
		ostringstream str;
		str << "r" << scheme.Id;
		reaction->SetId (str.str ().c_str ());
		state.loaded_ids[scheme.Id] = reaction->GetId ();
		// now, add the objects to the reaction
		for (i = scheme.Steps.begin (); i != iend; i++) {
			// first the arrow
			arrow = doc->GetChild ((state.loaded_ids[*((*i).Arrows.begin())]).c_str ());
			reaction->AddChild (arrow);
			// then reagents
			jend = (*i).Reagents.end ();
			parent = NULL;
			gcu::Object *rs = NULL; // make g++ happy
			for (j = (*i).Reagents.begin (); j != jend; j++) {
				obj = doc->GetDescendant (state.loaded_ids[*j].c_str ());
				if (obj == NULL) {
					delete reaction;
					return;
				}
				parent = obj->GetParent ();
				if (rs == NULL) {
					if (parent == doc) {
						rs = reaction->CreateObject ("reaction-step", reaction);
						arrow->SetProperty (GCU_PROP_ARROW_START_ID, rs->GetId ());
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else {
						rs = parent->GetParent ();
						if (rs->GetParent () != reaction) {
							delete reaction;
							return;
						}
					}
				} else {
					if (parent == doc) {
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else if (rs != parent->GetParent ()) {
						delete reaction;
						return;
					}
				}
				// search for potential stoichiometry coefficients
				arrow->SetProperty (GCU_PROP_ARROW_START_ID, rs->GetId ());
				rs->OnLoaded ();
			}
			// same treatment for products
			jend = (*i).Products.end ();
			rs = NULL;
			for (j = (*i).Products.begin (); j != jend; j++) {
				obj = doc->GetDescendant (state.loaded_ids[*j].c_str ());
				if (obj == NULL) {
					delete reaction;
					return;
				}
				parent = obj->GetParent ();
				if (rs == NULL) {
					if (parent == doc) {
						rs = reaction->CreateObject ("reaction-step", reaction);
						arrow->SetProperty (GCU_PROP_ARROW_END_ID, rs->GetId ());
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else {
						rs = parent->GetParent ();
						if (rs->GetParent () != reaction) {
							delete reaction;
							return;
						}
					}
				} else {
					if (parent == doc) {
						reactant = rs->CreateObject ("reactant", rs);
						reactant->SetProperty (GCU_PROP_MOLECULE, obj->GetId ());
					} else if (rs != parent->GetParent ()) {
						delete reaction;
						return;
					}
				}
				// search for potential stoichiometry coefficients
				arrow->SetProperty (GCU_PROP_ARROW_END_ID, rs->GetId ());
				rs->OnLoaded ();
			}
			// last, the objects attached above and below the arrow
			if (!(*i).ObjectsAbove.empty () || !(*i).ObjectsBelow.empty ()) {
				ostringstream str;
				str << (*i).ObjectsAbove.size ();
				arrow->SetProperty (GCU_PROP_REACTION_ARROW_MAX_LINES_ABOVE, str.str ().c_str ());
				str.str ("");
				unsigned n = 1;
				jend = (*i).ObjectsAbove.end ();
				for (j = (*i).ObjectsAbove.begin (); j != jend; j++) {
					obj = doc->GetDescendant (state.loaded_ids[*j].c_str ());
					if (obj == NULL) // we should emit at least a warning
						continue;
					parent = arrow->CreateObject ("reaction-prop", arrow);
					parent->SetProperty (GCU_PROP_ARROW_OBJECT, obj->GetId ());
					str << n;
					parent->SetProperty (GCU_PROP_REACTION_ARROW_PROP_LINE, str.str ().c_str ());
					str.str ("");
					n++;
				}
				jend = (*i).ObjectsBelow.end ();
				for (j = (*i).ObjectsBelow.begin (); j != jend; j++) {
					obj = doc->GetDescendant (state.loaded_ids[*j].c_str ());
					if (obj == NULL) // we should emit at least a warning
						continue;
					parent = arrow->CreateObject ("reaction-prop", arrow);
					parent->SetProperty (GCU_PROP_ARROW_OBJECT, obj->GetId ());
					str << n;
					parent->SetProperty (GCU_PROP_REACTION_ARROW_PROP_LINE, str.str ().c_str ());
					str.str ("");
					n++;
				}
			}
		}
		// now search for stoichiometry coefficients if any
		gcp::WidgetData *data = static_cast <gcp::Document * > (doc)->GetView ()->GetData ();
		gccv::Rect rect;
		double x0, y0, x1;
		std::map < std::string, Object * >::iterator k, l, r;
		std::pair <gcu::Object *, gcu::Object * > couple;
		std::list < std::pair <gcu::Object *, gcu::Object * > > couples;
		obj = doc->GetFirstChild (k);
		while (obj) {
			// assuming that only text object can be stoichiometric coefs
			if (obj->GetType () == gcu::TextType) {
				data->GetObjectBounds (obj, rect);
				x0 = rect.x0;
				y0 = (rect.y0 + rect.y1) / 2.;
				x1 = rect.x1 + state.padding;
				for (step = reaction->GetFirstChild (l); step; step = reaction->GetNextChild (l)) {
					if (gcu::Object::GetTypeName (step->GetType ()) != "reaction-step")
						continue;
					data->GetObjectBounds (step, rect);
					if (x0 > rect.x1 || x1 < rect.x0 || y0 > rect.y1 || y0 < rect.y0)
						continue;
					for (reactant = step->GetFirstChild (r); reactant; reactant = step->GetNextChild (r)) {
						if (reactant->GetType () != gcu::ReactantType)
							continue;
						data->GetObjectBounds (reactant, rect);
						if (x0 > rect.x0 || x1 < rect.x0 || y0 > rect.y1 || y0 < rect.y0)
							continue;
						// if we get there, we got it
						// we must not set it now to avoid an invalid iterator at this point, so store in couples.
						couple.first = reactant;
						couple.second = obj;
						couples.push_back (couple);
						goto next_text;
					}
				}
			}
next_text:
			obj = doc->GetNextChild (k);
		}
		std::list < std::pair <gcu::Object *, gcu::Object * > >::iterator c, cend = couples.end ();
		for (c = couples.begin (); c != cend; c++) {
			(*c).first->SetProperty (GCU_PROP_STOICHIOMETRY, (*c).second->GetId ());
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
	GSF_XML_IN_NODE (CDXML, PAGE, -1, "page", GSF_XML_CONTENT, cdxml_page_start, NULL),
		GSF_XML_IN_NODE (PAGE, T, -1, "t", GSF_XML_NO_CONTENT, cdxml_text_start, cdxml_text_end),
			GSF_XML_IN_NODE (T, S, -1, "s", GSF_XML_CONTENT, cdxml_string_start, cdxml_string_end),
		GSF_XML_IN_NODE (PAGE, FRAGMENT, -1, "fragment", GSF_XML_CONTENT, &cdxml_fragment_start, &cdxml_fragment_end),
			GSF_XML_IN_NODE (FRAGMENT, NODE, -1, "n", GSF_XML_CONTENT, cdxml_node_start, cdxml_simple_end),
				GSF_XML_IN_NODE (NODE, T, -1, "t", GSF_XML_2ND, NULL, NULL),
			GSF_XML_IN_NODE (FRAGMENT, BOND, -1, "b", GSF_XML_CONTENT, cdxml_bond_start, cdxml_simple_end),
		GSF_XML_IN_NODE (PAGE, GROUP, -1, "group", GSF_XML_CONTENT, cdxml_group_start, cdxml_simple_end),
			GSF_XML_IN_NODE (GROUP, FRAGMENT, -1, "fragment", GSF_XML_2ND, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, GRAPHIC, -1, "graphic", GSF_XML_CONTENT, cdxml_graphic_start, NULL),
		GSF_XML_IN_NODE (PAGE, ALTGROUP, -1, "altgroup", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, CURVE, -1, "curve", GSF_XML_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, STEP, -1, "step", GSF_XML_NO_CONTENT, NULL, NULL),
		GSF_XML_IN_NODE (PAGE, SCHEME, -1, "scheme", GSF_XML_NO_CONTENT, NULL, cdxml_scheme_end),
			GSF_XML_IN_NODE (SCHEME, REACTIONSTEP, -1, "step", GSF_XML_CONTENT, cdxml_step_start, NULL),
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

ContentType CDXMLLoader::Read  (Document *doc, GsfInput *in, G_GNUC_UNUSED char const *mime_type, GOIOContext *io)
{
	CDXMLReadState state;

	state.doc = doc;
	state.app = doc->GetApplication ();
	state.context = io;
	ContentType success = ContentTypeUnknown;
	state.colors.push_back ("red=\"0\" green=\"0\" blue=\"0\""); // black
	state.colors.push_back ("red=\"1\" green=\"1\" blue=\"1\""); // white
	state.font = 0;
	state.color = 0;
	state.theme = NULL;
	state.labelFont = -1;
	state.captionFont = -1;

	if (NULL != in) {
		GsfXMLInDoc *xml = gsf_xml_in_doc_new (cdxml_dtd, NULL);
		if (gsf_xml_in_doc_parse (xml, in, &state))
			success = ContentType2D;

		if (success == ContentTypeUnknown)
			go_io_warning (state.context,
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
					go_io_warning (state.context,
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
	// Update the view so that the positions are correct
	static_cast <gcp::Document *> (doc)->GetView ()->Update (doc);
	// now build schemes
	std::list < SchemeData >::iterator i, iend = state.schemes.end ();
	for (i = state.schemes.begin (); i != iend; i++) {
		if ((*i).Steps.empty ())
			continue;
		build_scheme (state, *i);
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

bool CDXMLLoader::WriteArrow (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	while (child) {
		if (!loader->WriteObject (xml, parent, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("graphic"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	std::istringstream in (obj->GetProperty (GCU_PROP_ARROW_COORDS));
	double x0, y0, x1, y1;
	in >> x0 >> y0 >> x1 >> y1;
	std::ostringstream out;
	out << x1 << " " << y1 << " " << x0 << " " << y0;
	AddStringProperty (node, "BoundingBox", out.str ());
	AddIntProperty (node, "Z", loader->m_Z++);
	AddStringProperty (node, "GraphicType", "Line");
	std::string type = gcu::Object::GetTypeName (obj->GetType ());
	if (type == "reaction-arrow")
		AddStringProperty (node, "ArrowType", (obj->GetProperty (GCU_PROP_REACTION_ARROW_TYPE) == "double")? "Equilibrium": "FullHead");
    else if (type == "mesomery-arrow")
		AddStringProperty (node, "ArrowType", "Resonance");
	else if (type == "retrosynthesis-arrow")
		AddStringProperty (node, "ArrowType", "RetroSynthesic");
	return true;
}

bool CDXMLLoader::WriteMesomery(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	std::list < gcu::Object const * > arrows;
	std::list < gcu::Object const * >::const_iterator it, itend;
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == "mesomery-arrow")
			arrows.push_back (child);
		else if (!loader->WriteObject (xml, parent, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	itend = arrows.end ();
	for (it = arrows.begin (); it != itend; it++)
		// save the graphic object.
		if (!WriteArrow (loader, xml, parent, *it, s))
			return false;
	// Now, save the mesomery
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("scheme"), NULL);
	xmlAddChild (parent, node);
	AddIntProperty (node, "id", loader->m_MaxId++);
	for (it = arrows.begin (); it != itend; it++) {
		// save the associated step
		xmlNodePtr step = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("step"), NULL);
		xmlAddChild (node, step);
		AddIntProperty (step, "id", loader->m_MaxId++);
		// reactants
		gcu::Object const *arrow = *it;
		gcu::Object const *cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_START_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			if (child) {
				std::ostringstream out;
				out << loader->m_SavedIds[child->GetId ()];
				AddStringProperty (step, "ReactionStepReactants", out.str ());
			}
		}
		// products
		cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_END_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			if (child) {
				std::ostringstream out;
				out << loader->m_SavedIds[child->GetId ()];
				AddStringProperty (step, "ReactionStepProducts", out.str ());
			}
		}
		// arrow
		AddIntProperty (step, "ReactionStepArrows", loader->m_SavedIds[arrow->GetId ()]);
	}
	return true;
}

bool CDXMLLoader::WriteReactionStep(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == "reaction-operator") {
			xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("graphic"), NULL);
			xmlAddChild (parent, node);
			loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
			AddIntProperty (node, "id", loader->m_MaxId++);
			// Write the bounding box
			std::istringstream str(child->GetProperty (GCU_PROP_POS2D));
			double x, y;
			str >> x >> y;
			x -= loader->m_FontSize / 3;
			y += loader->m_CHeight + loader->m_FontSize / 2;
			std::ostringstream out;
			out << x << " " << y - loader->m_FontSize << " " << x << " " << y;
			AddStringProperty (node, "BoundingBox", out.str ());
			AddIntProperty (node, "Z", loader->m_Z++);
			AddStringProperty (node, "GraphicType", "Symbol");
			AddStringProperty (node, "SymbolType", "Plus");
		} else if (!loader->WriteObject (xml, parent, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	return true;
}

bool CDXMLLoader::WriteReaction (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	std::list < gcu::Object const * > arrows;
	std::list < gcu::Object const * >::const_iterator it, itend;
	while (child) {
		std::string name = Object::GetTypeName (child->GetType ());
		if (name == "reaction-step") {
			if (!loader->WriteReactionStep (loader, xml, parent, child, s))
				return false;
		} else if (name == "reaction-arrow")
			arrows.push_back (child);
		child = obj->GetNextChild (i);
	}
	itend = arrows.end ();
	for (it = arrows.begin (); it != itend; it++)
		// save the graphic object.
		if (!WriteArrow (loader, xml, parent, *it, s))
			return false;
	// Now, save the reaction
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("scheme"), NULL);
	xmlAddChild (parent, node);
	AddIntProperty (node, "id", loader->m_MaxId++);
	for (it = arrows.begin (); it != itend; it++) {
		// save the associated step
		xmlNodePtr step = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const * > ("step"), NULL);
		xmlAddChild (node, step);
		AddIntProperty (step, "id", loader->m_MaxId++);
		std::list < unsigned > Ids, Ids_;
		// FIXME what happens if the reaction step contains a mesomery?
		// reactants
		gcu::Object const *arrow = *it;
		gcu::Object const *cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_START_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			while (child) {
				if (child->GetType () == gcu::ReactantType)
					Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_MOLECULE)]);
				child = cur->GetNextChild (i);
			}
			if (!Ids.empty ()) {
				std::ostringstream out;
				out << Ids.front ();
				Ids.pop_front ();
				while (!Ids.empty ()) {
					out << " " << Ids.front ();
					Ids.pop_front ();
				}
				AddStringProperty (step, "ReactionStepReactants", out.str ());
			}
		}
		// products
		cur = obj->GetDescendant (arrow->GetProperty (GCU_PROP_ARROW_END_ID).c_str ());
		if (cur) {
			child = cur->GetFirstChild (i);
			while (child) {
				if (child->GetType () == gcu::ReactantType)
					Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_MOLECULE)]);
				child = cur->GetNextChild (i);
			}
			if (!Ids.empty ()) {
				std::ostringstream out;
				out << Ids.front ();
				Ids.pop_front ();
				while (!Ids.empty ()) {
					out << " " << Ids.front ();
					Ids.pop_front ();
				}
				AddStringProperty (step, "ReactionStepProducts", out.str ());
			}
		}
		// arrow
		AddIntProperty (step, "ReactionStepArrows", loader->m_SavedIds[arrow->GetId ()]);
		// attached objects
		child = arrow->GetFirstChild (i);
		double y = arrow->GetYAlign ();
		while (child) {
			if (y > child->GetYAlign ())
				Ids_.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_ARROW_OBJECT)]);
			else
				Ids.push_back (loader->m_SavedIds[child->GetProperty (GCU_PROP_ARROW_OBJECT)]);
			child = arrow->GetNextChild (i);
		}
		// objects above the arrow
		if (!Ids.empty ()) {
			std::ostringstream out;
			out << Ids.front ();
			Ids.pop_front ();
			while (!Ids.empty ()) {
				out << " " << Ids.front ();
				Ids.pop_front ();
			}
			AddStringProperty (step, "ReactionStepObjectsAboveArrow", out.str ());
		}
		// objects below the arrow
		if (!Ids_.empty ()) {
			std::ostringstream out;
			out << Ids_.front ();
			Ids_.pop_front ();
			while (!Ids_.empty ()) {
				out << " " << Ids_.front ();
				Ids_.pop_front ();
			}
			AddStringProperty (step, "ReactionStepObjectsBelowArrow", out.str ());
		}
	}
	return true;
}

bool CDXMLLoader::WriteRetrosynthesis (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	std::map <std::string, Object *>::const_iterator i;
	gcu::Object const *child = obj->GetFirstChild (i);
	while (child) {
		// FIXME
		if (!loader->WriteObject (xml, parent, child, s))
			return false;
		child = obj->GetNextChild (i);
	}
	return true;
}

bool CDXMLLoader::WriteAtom (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("n"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	string prop = obj->GetProperty (GCU_PROP_POS2D);
	AddStringProperty (node, "p", prop);
	AddIntProperty (node, "Z", loader->m_Z++);
	prop = obj->GetProperty (GCU_PROP_ATOM_Z);
	if (prop != "6")
		AddStringProperty (node, "Element", prop);
	prop = obj->GetProperty (GCU_PROP_TEXT_TEXT);
	if (prop.length () > 0) {
		xmlNodePtr text = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("t"), NULL);
		xmlAddChild (node, text);
		string prop2 = obj->GetProperty (GCU_PROP_TEXT_POSITION);
		AddStringProperty (text, "p", prop2);
		AddStringProperty (text, "LabelJustification", "Left");
		AddStringProperty (text, "LabelAlignment", "Left");
		xmlNodePtr sub = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("s"), NULL);
		xmlAddChild (text, sub);
		AddIntProperty (sub, "font", loader->m_LabelFont);
		AddIntProperty (sub, "face", loader->m_LabelFontFace);
		AddIntProperty (sub, "size", loader->m_LabelFontSize);
		AddIntProperty (sub, "color", loader->m_LabelFontColor);
		xmlNodeAddContent (sub, reinterpret_cast <xmlChar const *> (prop.c_str ()));
	}
	return true;
}

bool CDXMLLoader::WriteFragment (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("n"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	std::string prop = obj->GetProperty (GCU_PROP_FRAGMENT_ATOM_ID);
	gcu::Object *atom = obj->GetChild (prop.c_str ());
	loader->m_SavedIds[atom->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	prop = obj->GetProperty (GCU_PROP_POS2D);
	AddStringProperty (node, "p", prop);
	AddIntProperty (node, "Z", loader->m_Z++);
	AddStringProperty (node, "NodeType", "Fragment");
	prop = obj->GetProperty (GCU_PROP_TEXT_TEXT);
	std::string pos = obj->GetProperty (GCU_PROP_FRAGMENT_ATOM_START);
	unsigned as = atoi (pos.c_str ());
	if (as > 0) {
		char const *symbol = static_cast < gcu::Atom * > (atom)->GetSymbol ();
		unsigned ae = as + strlen (symbol);
		if (ae < prop.length () - 1) {
			// attachment point is in the middle of the string, we need to bring it to start,
			// and add put the left part inside brackets
			std::string left = prop.substr (0, as), right = prop.substr (ae);
			prop = symbol;
			prop += "(";
			gcu::Formula *formula = new gcu::Formula (left, GCU_FORMULA_PARSE_RESIDUE);
			std::list < FormulaElt * > const &elts = formula->GetElements ();
			std::list< FormulaElt * >::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++)
				prop += (*i)->Text ();
			prop += ")";
			prop += right;
			delete formula;
		} else {
			// atom is at end, we need to revert the formula
			gcu::Formula *formula = new gcu::Formula (prop, GCU_FORMULA_PARSE_RESIDUE);
			std::list < FormulaElt * > const &elts = formula->GetElements ();
			prop.clear ();
			std::list< FormulaElt * >::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++)
				prop += (*i)->Text ();
			delete formula;
		}
	}
	if (prop.length () > 0) {
		xmlNodePtr text = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("t"), NULL);
		xmlAddChild (node, text);
		string prop2 = obj->GetProperty (GCU_PROP_TEXT_POSITION);
		AddStringProperty (text, "p", prop2);
		AddStringProperty (text, "LabelJustification", "Left");
		AddStringProperty (text, "LabelAlignment", "Left");
		xmlNodePtr sub = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("s"), NULL);
		xmlAddChild (text, sub);
		AddIntProperty (sub, "font", loader->m_LabelFont);
		AddIntProperty (sub, "face", loader->m_LabelFontFace);
		AddIntProperty (sub, "size", loader->m_LabelFontSize);
		AddIntProperty (sub, "color", loader->m_LabelFontColor);
		xmlNodeAddContent (sub, reinterpret_cast <xmlChar const *> (prop.c_str ()));
	}
	return true;
}

bool CDXMLLoader::WriteBond (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, G_GNUC_UNUSED GOIOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("b"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	AddIntProperty (node, "Z", loader->m_Z++);
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
	else if (prop == "large")
		prop = "Bold";
	else
		prop.clear ();
	if (prop.length ())
		AddStringProperty (node, "Display", prop);
	prop = obj->GetProperty (GCU_PROP_BOND_DOUBLE_POSITION);
	if (prop == "center")
		AddStringProperty (node, "DoublePosition", "Center");
	else if (prop == "right")
		AddStringProperty (node, "DoublePosition", "Right");
	else if (prop == "left")
		AddStringProperty (node, "DoublePosition", "Left");
	return true;
}

bool CDXMLLoader::WriteMolecule (CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("fragment"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	// save atoms
	std::map <std::string, Object *>::const_iterator i;
	Object const *child = obj->GetFirstChild (i);
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

bool CDXMLLoader::WriteNode (CDXMLLoader *loader, xmlNodePtr node, WriteTextState *state)
{
	std::string name (reinterpret_cast < char const * > (node->name));
	if (name == "br" || name == "text") {
		if (state->node == NULL) {
			state->node = xmlNewDocNode (state->xml, NULL, reinterpret_cast < xmlChar const * > ("s"),
			                             (name == "br")? reinterpret_cast < xmlChar const * > ("s\n"): xmlNodeGetContent (node));
			xmlAddChild (state->parent, state->node);
			AddIntProperty (state->node, "font", state->font);
			AddIntProperty (state->node, "size", state->size);
			AddIntProperty (state->node, "color", state->color);
			unsigned font_type = 0;
			if (state->bold)
				font_type |= 1;
			if (state->italic)
				font_type |= 2;
			if (state->underline)
				font_type |= 4;
			switch (state->position) {
			case -1:
				font_type |= 0x20;
				break;
			case 1:
				font_type |= 0x40;
				break;
			default:
				break;
			}
			if (font_type != 0)
				AddIntProperty (state->node, "face", font_type);
		} else
			xmlNodeAddContent (state->node, (name == "br")? reinterpret_cast < xmlChar const * > ("s\n"): xmlNodeGetContent (node));
		return true;
	} else
		state->node = NULL;
	WriteTextState child_state = *state;
	if (name == "i")
		child_state.italic = true;
	else if (name == "b")
		child_state.bold = true;
	else if (name == "u")
		child_state.underline = true;
	else if (name == "font") {
		xmlChar *buf = xmlGetProp (node, reinterpret_cast < xmlChar const * > ("name"));
		PangoFontDescription *desc = pango_font_description_from_string (reinterpret_cast < char * > (buf));
		xmlFree (buf);
		child_state.size = static_cast < double > (pango_font_description_get_size (desc)) / PANGO_SCALE;
		std::string family = pango_font_description_get_family (desc);
		if (family == "Arial")
			child_state.font = 3;
		else if (family == "Times New Roman")
			child_state.font = 4;
		else {
			guint16 i = 5;
			std::map < unsigned, CDXMLFont >::iterator it, itend = loader->m_Fonts.end ();
			for (it = loader->m_Fonts.find (i); it != itend; it++, i++)
				if (family == (*it).second.name)
					break;
			if (it == itend)
				loader->m_Fonts[i] = (CDXMLFont) {i, "iso-10646", family};
			child_state.font = i;
		}
	} else if (name == "sub")
		child_state.position = -1;
	else if (name == "sup")
		child_state.position = 1;
	else if (name == "fore") {
		GOColor color = ReadColor (node);
		guint i = 2;
		std::map < unsigned, GOColor >::iterator it, itend = loader->m_Colors.end ();
			for (it = loader->m_Colors.find (i); it != itend; it++, i++)
				if (color == (*it).second)
					break;
			if (it == itend)
				loader->m_Colors[i] = color;
		child_state.color = i;
		
	}
	xmlNodePtr child = node->children;
	while (child) {
		WriteNode (loader, child, &child_state);
		child = child->next;
	}
	return true;
}

bool CDXMLLoader::WriteText(CDXMLLoader *loader, xmlDocPtr xml, xmlNodePtr parent, Object const *obj, GOIOContext *s)
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("t"), NULL);
	xmlAddChild (parent, node);
	loader->m_SavedIds[obj->GetId ()] = loader->m_MaxId;
	AddIntProperty (node, "id", loader->m_MaxId++);
	std::string prop = obj->GetProperty (GCU_PROP_POS2D);
	if (prop.length ()) {
		istringstream str (prop);
		double x, y;
		str >> x >> y;
		y += loader->m_CHeight;
		std::ostringstream out;
		out << x << " " << y;
		AddStringProperty (node, "p", out.str ());
	}
	AddIntProperty (node, "Z", loader->m_Z++);
	// no need to export left justification or alignment.
	prop = obj->GetProperty (GCU_PROP_TEXT_ALIGNMENT);
	if (prop == "right")
		AddStringProperty (node, "Justification", "Right");
	else if (prop == "center")
		AddStringProperty (node, "Justification", "Center");
	prop = obj->GetProperty (GCU_PROP_TEXT_JUSTIFICATION);
	if (prop == "right")
		AddStringProperty (node, "CaptionJustification", "Right");
	else if (prop == "center")
		AddStringProperty (node, "CaptionJustification", "Center");
	else if (prop == "justify")
		AddStringProperty (node, "CaptionJustification", "Full");
	double inl;
	std::istringstream in (obj->GetProperty (GCU_PROP_TEXT_INTERLINE));
	in >> inl;
	if (inl <= 0.) {
		prop = obj->GetProperty (GCU_PROP_TEXT_VARIABLE_LINE_HEIGHT);
		AddStringProperty (node, "CaptionLineHeight", (prop =="true")? "variable": "auto");
	} else {
		std::istringstream in (obj->GetProperty (GCU_PROP_TEXT_MAX_LINE_HEIGHT));
		double lh;
		in >> lh;
		AddIntProperty (node, "CaptionLineHeight", inl + lh);
	}
	prop = obj->GetProperty (GCU_PROP_TEXT_MARKUP);
	xmlDocPtr doc = xmlParseMemory (prop.c_str(), prop.length ());
	xmlNodePtr text_node = doc->children->children;
	WriteTextState state;
	state.xml = xml;
	state.node = NULL;
	state.parent = node;
	state.s = s;
	state.italic = false;
	state.bold = false;
	state.underline = false;
	state.font = 3;
	state.size = 10;
	state.position = 0;
	state.color = 3;
	while (text_node) {
		if (strcmp (reinterpret_cast < char const *>(text_node->name), "position"))
			WriteNode (loader, text_node, &state);
		text_node = text_node->next;
	}
	xmlFreeDoc (doc);
	return true;
}

bool CDXMLLoader::WriteObject (xmlDocPtr xml, xmlNodePtr node, Object const *object, GOIOContext *io)
{
	string name = Object::GetTypeName (object->GetType ());
	map <string, bool (*) (CDXMLLoader *, xmlDocPtr, xmlNodePtr, Object const *, GOIOContext *)>::iterator i = m_WriteCallbacks.find (name);
	if (i != m_WriteCallbacks.end ())
		return (*i).second (this, xml, node, object, io);
	// if we don't save the object iself, try to save its children
	std::map <std::string, Object *>::const_iterator j;
	Object const *child = object->GetFirstChild (j);
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
	char *buf = g_strdup_printf ("%d", value);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (id), reinterpret_cast <xmlChar *> (buf));
	g_free (buf);
}
void CDXMLLoader::AddFloatProperty (xmlNodePtr node, char const *id, double value)
{
	std::ostringstream s;
	s << value;
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (id), reinterpret_cast <xmlChar const *> (s.str ().c_str ()));
}

void CDXMLLoader::AddStringProperty (xmlNodePtr node, char const *id, string const &value)
{
	xmlNewProp (node, reinterpret_cast <xmlChar const *> (id), reinterpret_cast <xmlChar const *> (value.c_str ()));
}

bool CDXMLLoader::Write  (Object const *obj, GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io, G_GNUC_UNUSED ContentType type)
{
	map<string, CDXMLFont> fonts;
	// FIXME: should be able to export a molecule or a reaction
	Document const *doc = dynamic_cast <Document const *> (obj);
	xmlNodePtr colors, fonttable, page;
	if (!doc || !out)
		return false;

	m_MaxId = m_Z = 1;

	// Init default colors
	m_Colors[2] = GO_COLOR_WHITE;
	m_Colors[3] = GO_COLOR_BLACK;
	m_Colors[4] = GO_COLOR_RED;
	m_Colors[5] = GO_COLOR_YELLOW;
	m_Colors[6] = GO_COLOR_GREEN;
	m_Colors[7] = GO_COLOR_CYAN;
	m_Colors[8] = GO_COLOR_BLUE;
	m_Colors[9] = GO_COLOR_VIOLET;

	// Init fonts, we always use Unknown as the charset, hoping it is not an issue
	m_Fonts[3] = (CDXMLFont) {3, string ("iso-8859-1"), string ("Arial")};
	m_Fonts[4] = (CDXMLFont) {4, string ("iso-8859-1"), string ("Times New Roman")};
	m_LabelFont = 3;
	m_LabelFontSize = 10.;

	/* we can't use sax, because we need colors and fonts */
	xmlDocPtr xml = xmlNewDoc (reinterpret_cast <xmlChar const *> ("1.0"));
	xmlDocSetRootElement (xml,  xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("CDXML"), NULL));
	std::string str = doc->GetApp ()->GetName () + " "VERSION;
	xmlNewProp (xml->children, reinterpret_cast <xmlChar const *> ("CreationProgram"),
	            reinterpret_cast <xmlChar const *> (str.c_str ()));
	// Get the theme (we need a gcp::Document there)
	gcp::Document const *cpDoc = dynamic_cast < gcp::Document const * > (doc);
	gcp::Theme const *theme = cpDoc->GetTheme ();
	m_Zoom = 1. / theme->GetZoomFactor();
	m_Scale = .75 / m_Zoom;
	m_CHeight = const_cast < gcp::Document * > (cpDoc)->GetView ()->GetCHeight ();
	// save title authors and the like
	str = doc->GetProperty (GCU_PROP_DOC_CREATOR);
	if (str.length () > 0)
		AddStringProperty (xml->children, "CreationUserName", str);
/* Don't write dates for now since the needed format is unknown
	str = doc->GetProperty (GCU_PROP_DOC_CREATION_TIME);
	unsigned d, m, Y;
	if (sscanf (str.c_str (), "%u-%u-%u", &Y,&m, &d) == 3) {
		GDateTime *date = g_date_time_new_utc (Y, m, d, 0, 0, 0.);
		AddDateProperty (out, "CreationDate", date);
		g_date_time_unref (date);
	}
	GDateTime *dt = g_date_time_new_now_utc ();
	AddDateProperty (out, "ModificationDate", dt);
	g_date_time_unref (dt);
*/
	str = doc->GetProperty (GCU_PROP_DOC_TITLE);
	if (str.length () > 0)
		AddStringProperty (xml->children, "Name", str);
	str = doc->GetProperty (GCU_PROP_DOC_COMMENT);
	if (str.length () > 0)
		AddStringProperty (xml->children, "Comment", str);
	// determine the bond length and scale the document appropriately
	const_cast <Document *> (doc)->SetScale (1. / m_Scale);
	double l = theme->GetBondLength () * m_Scale;
	AddFloatProperty (xml->children, "BondLength", l);
	int n = theme->GetBondDist () * 100. * m_Zoom / theme->GetBondLength ();
	AddIntProperty (xml->children, "BondSpacing", n);
	l = theme->GetBondWidth () * .75;
	AddFloatProperty (xml->children, "LineWidth", l);
	l = theme->GetStereoBondWidth () * .75;
	AddFloatProperty (xml->children, "BoldWidth", l);
	l = theme->GetHashDist () * .75;
	AddFloatProperty (xml->children, "HashSpacing", l);
	AddFloatProperty (xml->children, "ChainAngle", theme->GetBondAngle ());
	l = theme->GetPadding () * .75;
	AddFloatProperty (xml->children, "MarginWidth", l);
	str = theme->GetTextFontFamily ();
	if (str == "Arial")
		n = 3;
	else if (str == "Times New Roman")
		n = 4;
	else {
		n = 5;
		std::map < unsigned, CDXMLFont >::iterator it, itend = m_Fonts.end ();
		for (it = m_Fonts.find (n); it != itend; it++, n++)
			if (str == (*it).second.name)
				break;
		if (it == itend)
			m_Fonts[n] = (CDXMLFont) {static_cast < guint16 > (n), "iso-10646", str};
	}
	AddIntProperty (xml->children, "CaptionFont", n);
	n = theme->GetTextFontSize () / PANGO_SCALE;
	m_FontSize = n * .75;
	AddIntProperty (xml->children, "CaptionSize", n);
	n = 0;
	if (theme->GetTextFontWeight () > PANGO_WEIGHT_NORMAL)
		n |= 1;
	if (theme->GetTextFontStyle () != PANGO_STYLE_NORMAL)
		n |= 2;
	AddIntProperty (xml->children, "CaptionFace", n);
	str = theme->GetFontFamily ();
	if (str == "Arial")
		m_LabelFont = 3;
	else if (str == "Times New Roman")
		m_LabelFont = 4;
	else {
		m_LabelFont = 5;
		std::map < unsigned, CDXMLFont >::iterator it, itend = m_Fonts.end ();
		for (it = m_Fonts.find (m_LabelFont); it != itend; it++, m_LabelFont++)
			if (str == (*it).second.name)
				break;
		if (it == itend)
			m_Fonts[m_LabelFont] = (CDXMLFont) {static_cast < guint16 > (m_LabelFont), "iso-10646", str};
	}
	AddIntProperty (xml->children, "LabelFont", m_LabelFont);
	m_LabelFontSize = theme->GetFontSize () / PANGO_SCALE;
	AddIntProperty (xml->children, "LabelSize", m_LabelFontSize);
	m_LabelFontFace = 0x60;
	if (theme->GetFontWeight () > PANGO_WEIGHT_NORMAL)
		m_LabelFontFace |= 1;
	if (theme->GetFontStyle () != PANGO_STYLE_NORMAL)
		m_LabelFontFace |= 2;
	AddIntProperty (xml->children, "LabelFace", m_LabelFontFace);
	m_LabelFontColor = 0;
	AddIntProperty (xml->children, "CaptionJustification", 0);
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
	std::map <std::string, Object *>::const_iterator i;
	Object const *child = doc->GetFirstChild (i);
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
		WriteFloat (node, "r", GO_COLOR_DOUBLE_R ((*color).second));
		WriteFloat (node, "g", GO_COLOR_DOUBLE_G ((*color).second));
		WriteFloat (node, "b", GO_COLOR_DOUBLE_B ((*color).second));
	}
	// write fonts
	map <unsigned, CDXMLFont>::iterator font, end_font = m_Fonts.end ();
	for (font = m_Fonts.begin (); font != end_font; font++) {
		xmlNodePtr node = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("font"), NULL);
		xmlAddChild (fonttable, node);
		WriteInt (node, "id", (*font).second.index);
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
