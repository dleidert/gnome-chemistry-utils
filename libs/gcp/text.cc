// -*- C++ -*-

/* 
 * GChemPaint library
 * text.cc 
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "text.h"
#include "widgetdata.h"
#include "view.h"
#include "document.h"
#include "application.h"
#include "settings.h"
#include "theme.h"
#include "tool.h"
#include "window.h"
#include <gccv/canvas.h>
#include <gccv/text.h>
#include <gcu/formula.h>
#include <gcu/objprops.h>
#include <glib/gi18n-lib.h>
#include <stdexcept>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

Text::Text ():
	TextObject (TextType),
	m_Align (PANGO_ALIGN_LEFT),
	m_Justified (false),
	m_Anchor (gccv::AnchorLineWest)
{
}

Text::Text (double x, double y):
	TextObject (x, y, TextType),
	m_Align (PANGO_ALIGN_LEFT),
	m_Justified (false),
	m_Anchor (gccv::AnchorLineWest)
{
}

Text::~Text ()
{
}

void Text::GetCoords (double *x, double *y)
{
	*x = m_x;
	*y = m_y;
}

void Text::SetCoords (double x, double y)
{
	m_x = x;
	m_y = y;
}

class SaveStruct {
public:
	SaveStruct (gccv::TextTag *tag, unsigned start, unsigned end);
	~SaveStruct ();

	void Filter (SaveStruct **cur_state) throw (std::logic_error);
	bool Save (xmlDocPtr xml, xmlNodePtr node, unsigned &index, string const &text, unsigned es, unsigned ef, char const *f, double n);

	SaveStruct *next, *children;
	gccv::TextTag *m_tag;
	unsigned m_start, m_end;
};

SaveStruct::SaveStruct (gccv::TextTag *tag, unsigned start, unsigned end)
{
	m_tag = tag;
	m_start = start;
	m_end = end;
	next = children = NULL;
}

SaveStruct::~SaveStruct ()
{
	if (children)
		delete children;
	if (next)
		delete next;
}

void SaveStruct::Filter (SaveStruct **cur_state) throw (std::logic_error)
{
	if (!*cur_state) {
		*cur_state = this;
		return;
	}
	if (m_start < (*cur_state)->m_start)
		throw logic_error (_("This should not have occured, please file a bug record."));
	else if (m_start == (*cur_state)->m_start) {
		if (m_end <= (*cur_state)->m_end)
			Filter (&(*cur_state)->children);
		else {
			if ((*cur_state)->next) {
				throw  logic_error (_("This should not have occured, please file a bug record."));
			} else {
				// in that case, just set the new tag as parent of the old one
				SaveStruct *s = *cur_state;
				*cur_state = this;
				(*cur_state)->children = s;
			}
		}
	} else {
		if (m_start >= (*cur_state)->m_end)
			Filter (&(*cur_state)->next);
		else if (m_end <= (*cur_state)->m_end)
			Filter (&(*cur_state)->children);
		else {
			SaveStruct *s = new SaveStruct (m_tag, (*cur_state)->m_end, m_end);
			m_end = (*cur_state)->m_end;
			Filter (&(*cur_state)->children);
			s->Filter (&(*cur_state)->next);
		}
	}
}

bool SaveStruct::Save (xmlDocPtr xml, xmlNodePtr node, unsigned &index, string const &text, unsigned es, unsigned ef, char const *f, double n)
{
	xmlNodePtr child = NULL;
	if (index < m_start) {
		xmlNodeAddContentLen (node, reinterpret_cast <xmlChar const *> (text.c_str () + index), m_start - index);
		index = m_start;
	}
	switch (m_tag->GetTag ()) {
	case gccv::Family:
		f = static_cast <gccv::FamilyTextTag *> (m_tag)->GetFamily ().c_str ();
		ef = m_end;
		if (es >= ef) {
			char *str = g_strdup_printf ("%s %g", f, (double) n / PANGO_SCALE);
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("font"), NULL);
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("name"), reinterpret_cast <xmlChar const *> (str));
			g_free (str);
		}
		break;
	case gccv::Size:
		n = static_cast <gccv::SizeTextTag *> (m_tag)->GetSize ();
		es = m_end;
		if (ef >= es) {
			char *str = g_strdup_printf ("%s %g", f, (double) n / PANGO_SCALE);
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("font"), NULL);
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("name"), reinterpret_cast <xmlChar const *> (str));
			g_free (str);
		}
		break;
	case gccv::Style: {
		PangoStyle st = static_cast <gccv::StyleTextTag *> (m_tag)->GetStyle ();
		if (st == PANGO_STYLE_NORMAL)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("i"), NULL);
		if (st == PANGO_STYLE_OBLIQUE)
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("style"), reinterpret_cast <xmlChar const *> ("oblique"));
		break;
	}
	case gccv::Weight: {
		PangoWeight w = static_cast <gccv::WeightTextTag *> (m_tag)->GetWeight ();
		if (w == PANGO_WEIGHT_NORMAL)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("b"), NULL);
		if (w != PANGO_WEIGHT_BOLD) {
			char *buf = g_strdup_printf ("%d", (int) w / 100);
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("weight"), reinterpret_cast <xmlChar const *> (buf));
			g_free (buf);
		}
		break;
	}
	case gccv::Variant: {
		PangoVariant v = static_cast <gccv::VariantTextTag *> (m_tag)->GetVariant ();
		if (v == PANGO_VARIANT_SMALL_CAPS)
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("small-caps"), NULL);
		break;
	}
	case gccv::Stretch: {
		PangoStretch st = static_cast <gccv::StretchTextTag *> (m_tag)->GetStretch ();
		char const *stretch = NULL;
		switch (st) {
		case PANGO_STRETCH_ULTRA_CONDENSED:
			stretch = "ultra-condensed";
			break;
		case PANGO_STRETCH_EXTRA_CONDENSED:
			stretch = "extra-condensed";
			break;
		case PANGO_STRETCH_CONDENSED:
			stretch = "condensed";
			break;
		case PANGO_STRETCH_SEMI_CONDENSED:
			stretch = "semi-condensed";
			break;
		case PANGO_STRETCH_NORMAL:
			break;
		case PANGO_STRETCH_SEMI_EXPANDED:
			stretch = "semi-expanded";
			break;
		case PANGO_STRETCH_EXPANDED:
			stretch = "expanded";
			break;
		case PANGO_STRETCH_EXTRA_EXPANDED:
			stretch = "extra-expanded";
			break;
		case PANGO_STRETCH_ULTRA_EXPANDED:
			stretch = "ultra-expanded";
			break;
		}
		if (!stretch)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("stretch"), NULL);
		xmlNewProp (child, reinterpret_cast <xmlChar  const *> ("type"), reinterpret_cast <xmlChar  const *> (stretch));
		break;
	}
	case gccv::Underline: {
		PangoUnderline u = static_cast <gccv::UnderlineTextTag *> (m_tag)->GetUnderline ();
		char const *type = NULL;
		switch (u) {
		case PANGO_UNDERLINE_NONE:
		case PANGO_UNDERLINE_SINGLE:
			break;
		case PANGO_UNDERLINE_DOUBLE:
			type = "double";
			break;
		case PANGO_UNDERLINE_LOW:
			type = "low";
			break;
		case PANGO_UNDERLINE_ERROR:
			type = "error"; // not really implemented
			break;
		}
		if (u == PANGO_UNDERLINE_NONE)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("u"), NULL);
		if (u != PANGO_UNDERLINE_SINGLE)
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
		break;
	}
	case gccv::Strikethrough:
		if (static_cast <gccv::StrikethroughTextTag *> (m_tag)->GetStrikethrough ())
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("s"), NULL);
		break;
	case gccv::Foreground: {
		GOColor color = static_cast <gccv::ForegroundTextTag *> (m_tag)->GetColor ();
		if (color == RGBA_BLACK)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("fore"), NULL);
		char *buf = g_strdup_printf ("%g", DOUBLE_RGBA_R (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("red"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_G (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("green"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_B (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("blue"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		if (UINT_RGBA_A (color) != 0xff) {
			buf = g_strdup_printf ("%g", DOUBLE_RGBA_A (color));
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("alpha"), reinterpret_cast <xmlChar const *> (buf));
			g_free (buf);
		}
		break;
	}
	case gccv::Background: {
		GOColor color = static_cast <gccv::BackgroundTextTag *> (m_tag)->GetColor ();
		if (color == RGBA_BLACK)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("back"), NULL);
		char *buf = g_strdup_printf ("%g", DOUBLE_RGBA_R (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("red"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_G (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("green"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		buf = g_strdup_printf ("%g", DOUBLE_RGBA_B (color));
		xmlNewProp (child, reinterpret_cast <xmlChar const *> ("blue"), reinterpret_cast <xmlChar const *> (buf));
		g_free (buf);
		if (UINT_RGBA_A (color) != 0xff) {
			buf = g_strdup_printf ("%g", DOUBLE_RGBA_A (color));
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("alpha"), reinterpret_cast <xmlChar const *> (buf));
			g_free (buf);
		}
		break;
	}
	case gccv::Rise: {
		double rise = static_cast <gccv::RiseTextTag *> (m_tag)->GetRise ();
		if (rise != 0) {
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ((rise > 0)? "sup": "sub"), NULL);
			char *buf = g_strdup_printf ("%g", fabs (rise));
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("height"), reinterpret_cast <xmlChar const *> (buf));
			g_free (buf);
			xmlAddChild (node, child);
		}
		break;
	}
	case gccv::Position: {
		bool stacked;
		double size;
		gccv::TextPosition pos = static_cast <gccv::PositionTextTag *> (m_tag)->GetPosition (stacked, size);
		switch (pos) {
		case gccv::Subscript:
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("sub"), NULL);
			break;
		case gccv::Superscript:
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("sup"), NULL);
			break;
		default:
			break;
		}
		if (child) {
			if (stacked)
				xmlNewProp (child, reinterpret_cast <xmlChar const *> ("stacked"), reinterpret_cast <xmlChar const *> ("true"));
			if (size != n) {
				char *buf = g_strdup_printf ("%g", fabs (size));
				xmlNewProp (child, reinterpret_cast <xmlChar const *> ("size"), reinterpret_cast <xmlChar const *> (buf));
				g_free (buf);
			}
		}
		break;
	}
	default:
		break;
	}
	if (!child)
		child = node;
	else
		xmlAddChild (node, child);
	if (children)
		if (!children->Save (xml, child, index, text, es, ef, f, n))
			return false;
	
	if (index < m_end) {
		xmlNodeAddContentLen (child, reinterpret_cast <xmlChar const *> (text.c_str () + index), m_end - index);
		index = m_end;
	}
	if (next)
		if (!next->Save (xml, node, index, text, es, ef, f, n))
			return false;
	return true;
}

static bool tag_order (gccv::TextTag *first, gccv::TextTag *last)
{
	if (first->GetStartIndex () < last->GetStartIndex ())
		return true;
	else if (first->GetStartIndex () > last->GetStartIndex ())
		return false;
	if (first->GetEndIndex () > last->GetEndIndex ())
		return true;
	else if (first->GetEndIndex () < last->GetEndIndex ())
		return false;
	return first->GetTag () < last->GetTag ();
}

xmlNodePtr Text::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "text", NULL);
	if (!node)
		return NULL;
	if (!TextObject::SaveNode (xml, node)) {
		xmlFreeNode (node);
		return NULL;
	}
	switch (m_Anchor) {
	case gccv::AnchorLineWest:
		xmlNewProp (node, (xmlChar const *) "anchor", (xmlChar const *) "right");
		break;
	case gccv::AnchorLine:
		xmlNewProp (node, (xmlChar const *) "anchor", (xmlChar const *) "center");
		break;
	default:
		break;
	}
	if (m_Justified)
		xmlNewProp (node, (xmlChar const *) "justification", (xmlChar const *) "justify");
	else if (m_Align != PANGO_ALIGN_LEFT)
		xmlNewProp (node, (xmlChar const *) "justification", (xmlChar const *) ((m_Align == PANGO_ALIGN_RIGHT)? "right": "center"));
	unsigned i = 0;
	SaveStruct *head = NULL, *cur;
	char const *text;
	std::list <gccv::TextTag *> const *tags;
	if (m_Item) {
		gccv::Text *t = reinterpret_cast <gccv::Text *> (m_Item);
		text = t->GetText ();
		tags = t->GetTags ();
	} else {
		text = m_buf.c_str ();
		tags = &m_TagList;
	}
	gccv::TextTagList tt; // the tags in this list will be destroyed on return
	std::list <gccv::TextTag *>::const_iterator j, jend = tags->end ();
	// duplicate the tags so that they can be sorted
	for (j = tags->begin (); j != jend; j++) {
		gccv::TextTag *tag = (*j)->Duplicate ();
		tag->SetStartIndex ((*j)->GetStartIndex ());
		tag->SetEndIndex ((*j)->GetEndIndex ());
		tt.push_back (tag);
	}
	// sort the duplicated tags
	tt.sort (tag_order);
	// now save the text and tags
	// first buid the tags tree
	gccv::TextTagList::iterator k, kend = tt.end ();
	for (k = tt.begin (); k != kend; k++) {
		cur = new SaveStruct (*k, (*k)->GetStartIndex (), (*k)->GetEndIndex ());
		cur->Filter (&head);
	}
	// now, save the tree
	if (head)
		head->Save (xml, node, i, m_buf, 0, 0, NULL, 0);
	xmlNodeAddContent (node, reinterpret_cast <xmlChar const *> (m_buf.c_str () + i));
	delete head;
	return node;
}

struct SelState {
	unsigned start, end;
	PangoAttrList *l;
};

bool selection_filter_func (PangoAttribute *attribute, struct SelState *state)
{
	if (attribute->start_index < state->end && attribute ->end_index > state->start) {
		PangoAttribute *attr = pango_attribute_copy (attribute);
		attr->start_index = (attribute->start_index < state->start)? 0: attribute->start_index - state->start;
		attr->end_index = (attribute->end_index > state->end)? state->end - state->start: attribute->end_index - state->start;
		pango_attr_list_insert (state->l, attr);
	}
	return false;
}

xmlNodePtr Text::SaveSelection (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "text", NULL);
	if (!node)
		return NULL;
	// get the text and attributes
/*	const char *text = pango_layout_get_text (m_Layout);
	PangoAttrList *l = pango_layout_get_attributes (m_Layout);
	string selection (text + m_StartSel, m_EndSel - m_StartSel);
	// make a new filtered attributes list
	struct SelState state;
	state.start = m_StartSel;
	state.end = m_EndSel;
	state.l = pango_attr_list_new ();
	pango_attr_list_filter (l, (PangoAttrFilterFunc) selection_filter_func, &state);
	unsigned i = 0;
	SaveStruct *head = NULL, *cur;
	pango_attr_list_filter (state.l, (PangoAttrFilterFunc) filter_func, &head);
	cur = head;
	while (cur) {
		save_state (xml, node, selection.c_str (), cur, i, 0, 0, NULL, 0);
		i = cur->attr->end_index;
		cur = cur->next;
	}
	delete head;
	pango_attr_list_unref (state.l);*/
	return (TextObject::SaveNode (xml, node))? node: NULL;
}

bool Text::Load (xmlNodePtr node)
{
	if (!TextObject::Load (node))
		return false;
	xmlChar *buf = xmlGetProp (node, (xmlChar const *) "justification");
	if (buf) {
		if (!strcmp ((char const *) buf, "justify"))
			m_Justified = true;
		else if (!strcmp ((char const *) buf, "right"))
			m_Justified = PANGO_ALIGN_RIGHT;
		else if (!strcmp ((char const *) buf, "center"))
			m_Justified = PANGO_ALIGN_CENTER;
		else
			m_Justified = PANGO_ALIGN_LEFT;
		xmlFree (buf);
	}
	buf = xmlGetProp (node, (xmlChar const *) "anchor");
	if (buf) {
		if (!strcmp ((char const *) buf, "right"))
			m_Anchor = gccv::AnchorLineEast;
		else if (!strcmp ((char const *) buf, "center"))
			m_Anchor = gccv::AnchorLine;
		else
			m_Anchor = gccv::AnchorLineWest;
		xmlFree (buf);
	}
	xmlNodePtr child;
	m_bLoading = true;
	child = node->children;
	m_buf.clear ();
	unsigned pos = 0;
	while (child) {
		if (!LoadNode (child, pos, 1))
			return false;
		child = child->next;
	}
	if (m_Item) {
		gccv::Text *text = reinterpret_cast <gccv::Text *> (m_Item);
		text->SetText (m_buf.c_str ());
		while (!m_TagList.empty ()) {
			text->InsertTextTag (m_TagList.front ());
			m_TagList.pop_front ();
		}
	}
	m_bLoading = false;
	return true;
}

bool Text::LoadSelection (xmlNodePtr node, unsigned pos)
{
	xmlNodePtr child;
	m_bLoading = true;
/*	m_buf = pango_layout_get_text (m_Layout);
	m_AttrList = pango_layout_get_attributes (m_Layout);
	child = node->children;
	while (child) {
		if (!LoadNode (child, pos, 1))
			return false;
		child = child->next;
	}
	pango_layout_set_text (m_Layout, m_buf.c_str (), -1);
	pango_layout_set_attributes (m_Layout, m_AttrList);
	GtkWidget* pWidget = dynamic_cast<Document*> (GetDocument ())->GetWidget ();
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
	GnomeCanvasGroup *group = pData->Items[this];
	if (group) {
		GnomeCanvasPango *PangoItem = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (group), "text"));
		gnome_canvas_pango_set_selection_bounds (PangoItem, pos, pos);
	}	*/
	m_bLoading = false;
//	OnChanged (true);
	return true;
}

struct limits {
	unsigned start, length;
};

static bool on_insert (PangoAttribute *attr, struct limits *l)
{
	if (attr->start_index > l->start) {
		attr->start_index += l->length;
		attr->end_index += l->length;
	} else if (attr->end_index > l->start)
		attr->end_index += l->length;
	return false;
}

bool Text::LoadNode (xmlNodePtr node, unsigned &pos, int level, int cur_size)
{
	char* buf;
	gccv::TextTag *tag = NULL, *tag0 = NULL;
	unsigned start = pos;
	if (!strcmp ((const char*) node->name, "text")) {
		if (!level)
			return true;
		buf = (char*) xmlNodeGetContent (node);
		if (buf) {
			pos += strlen (buf);
			m_buf.insert (start, buf);
			xmlFree (buf);
		}
	} else if (!strcmp ((const char*) node->name, "br")) {
/* FIXME		m_buf.insert (pos, "\n");
		pos++;
		struct limits l;
		l.start = start;
		l.length = 1;
		pango_attr_list_filter (m_AttrList, (PangoAttrFilterFunc) on_insert, &l);*/
	} else if (!strcmp ((const char*) node->name, "b")) {
		PangoWeight weight = PANGO_WEIGHT_BOLD;
		buf = (char*) xmlGetProp(node, (xmlChar*) "weight");
		if (buf) {
			weight = (PangoWeight) (strtol (buf, NULL, 10) * 100);
			xmlFree (buf);
		}
		tag = new gccv::WeightTextTag (weight);
	} else if (!strcmp ((const char*) node->name, "i")) {
		PangoStyle style = PANGO_STYLE_ITALIC;
		buf = (char*) xmlGetProp(node, (xmlChar*) "style");
		if (buf) {
			if (!strcmp (buf, "oblique"))
				style = PANGO_STYLE_OBLIQUE;
			xmlFree (buf);
		}
		tag = new gccv::StyleTextTag (style);
	} else if (!strcmp ((const char*) node->name, "u")) {
		PangoUnderline underline = PANGO_UNDERLINE_SINGLE;
		buf = (char*) xmlGetProp(node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double"))
				underline = PANGO_UNDERLINE_DOUBLE;
			else if (!strcmp (buf, "low"))
				underline = PANGO_UNDERLINE_LOW;
			else if (!strcmp (buf, "error"))
				underline = PANGO_UNDERLINE_ERROR;
			xmlFree (buf);
		}
		tag = new gccv::UnderlineTextTag (underline);
	} else if (!strcmp ((const char*) node->name, "s"))
		tag = new gccv::StrikethroughTextTag (true);
	else if (!strcmp ((const char*) node->name, "sub")) {
		buf = (char*) xmlGetProp (node, (xmlChar*) "height");
		if (buf) {
			int rise = -strtoul (buf, NULL, 10) * PANGO_SCALE;
			xmlFree (buf);
			tag = new gccv::RiseTextTag (rise);
		} else {
			bool stacked = false;
			double size = cur_size;
			buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("stacked")));
			if (buf) {
				stacked = !strcmp (buf, "true");
				xmlFree (buf);
			}
			buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("size")));
			if (buf) {
				size = strtod (buf, NULL);
				xmlFree (buf);
			}
			tag = new gccv::PositionTextTag (gccv::Subscript, stacked, size);
		}
	} else if (!strcmp ((const char*) node->name, "sup")) {
		buf = (char*) xmlGetProp (node, (xmlChar*) "height");
		if (buf) {
			int rise = strtoul (buf, NULL, 10) * PANGO_SCALE;
			xmlFree (buf);
			tag = new gccv::RiseTextTag (rise);
		} else {
			bool stacked = false;
			double size = cur_size;
			buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("stacked")));
			if (buf) {
				stacked = !strcmp (buf, "true");
				xmlFree (buf);
			}
			buf = reinterpret_cast <char *> (xmlGetProp (node, reinterpret_cast <xmlChar const *> ("size")));
			if (buf) {
				size = strtod (buf, NULL);
				xmlFree (buf);
			}
			tag = new gccv::PositionTextTag (gccv::Superscript, stacked, size);
		}
	} else if (!strcmp ((const char*) node->name, "font")) {
		char* TagName = (char*) xmlGetProp (node, (xmlChar*) "name");
		if (!TagName)
			return false;
		PangoFontDescription* pfd = pango_font_description_from_string (TagName);
		tag = new gccv::FamilyTextTag (pango_font_description_get_family (pfd));
		cur_size = pango_font_description_get_size (pfd);
		tag0 = new gccv::SizeTextTag (cur_size);
		pango_font_description_free (pfd);
		xmlFree (TagName);
	} else if (!strcmp ((const char*) node->name, "small-caps"))
		tag = new gccv::VariantTextTag (PANGO_VARIANT_SMALL_CAPS);
	else if (!strcmp ((const char*) node->name, "stretch")) {
		buf = (char*) xmlGetProp(node, (xmlChar*) "type");
		if (!buf)
			return false;
		PangoStretch stretch = PANGO_STRETCH_NORMAL;
		if (!strcmp (buf, "ultra-condensed"))
			stretch = PANGO_STRETCH_ULTRA_CONDENSED;
		else if(!strcmp (buf, "extra-condensed"))
			stretch = PANGO_STRETCH_EXTRA_CONDENSED;
		else if(!strcmp (buf, "condensed"))
			stretch = PANGO_STRETCH_CONDENSED;
		else if(!strcmp (buf, "semi-condensed"))
			stretch = PANGO_STRETCH_SEMI_CONDENSED;
		else if(!strcmp (buf, "semi-expanded"))
			stretch = PANGO_STRETCH_SEMI_EXPANDED;
		else if(!strcmp (buf, "expanded"))
			stretch = PANGO_STRETCH_EXPANDED;
		else if(!strcmp (buf, "extra-expanded"))
			stretch = PANGO_STRETCH_EXTRA_EXPANDED;
		else if(!strcmp (buf, "ultra-expanded"))
			stretch = PANGO_STRETCH_ULTRA_EXPANDED;
		xmlFree (buf);
		tag = new gccv::StretchTextTag (stretch);
	} else if (!strcmp ((const char*) node->name, "fore")) {
		guint16 red, green, blue;
		buf = (char*) xmlGetProp(node, (xmlChar*) "red");
		if (!buf)
			return false;
		red = (guint16) (strtod (buf, NULL) * 0xffff);
		xmlFree (buf);
		buf = (char*) xmlGetProp(node, (xmlChar*) "green");
		if (!buf)
			return false;
		green = (guint16) (strtod (buf, NULL) * 0xffff);
		xmlFree (buf);
		buf = (char*) xmlGetProp(node, (xmlChar*) "blue");
		if (!buf)
			return false;
		blue = (guint16) (strtod (buf, NULL) * 0xffff);
		xmlFree (buf);
		tag = new gccv::ForegroundTextTag (RGB_TO_UINT (red, green, blue));
	} else
		return true;
	xmlNodePtr child = node->children;
	while (child) {
		if (!LoadNode (child, pos, 1, cur_size))
			return false;
		child = child->next;
	}
	if (tag) {
		tag->SetStartIndex (start);
		tag->SetEndIndex (pos);
		if (tag->GetPriority () == gccv::TagPriorityLast)
			m_TagList.push_back (tag);
		else
			m_TagList.push_front (tag);
	}
	if (tag0) {
		tag0->SetStartIndex (start);
		tag0->SetEndIndex (pos);
		if (tag0->GetPriority () == gccv::TagPriorityLast)
			m_TagList.push_back (tag0);
		else
			m_TagList.push_front (tag0);
	}
	return true;
}

void Text::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	PangoFontDescription *desc = pango_font_description_new ();
	pango_font_description_set_family (desc, doc->GetTextFontFamily ());
	pango_font_description_set_style (desc, doc->GetTextFontStyle ());
	pango_font_description_set_variant (desc, doc->GetTextFontVariant ());
	pango_font_description_set_weight (desc, doc->GetTextFontWeight ());
	pango_font_description_set_size (desc, doc->GetTextFontSize ());
	if (m_ascent <= 0) {
		PangoContext* pc = gccv::Text::GetContext ();
		PangoLayout *layout = pango_layout_new (pc);
		pango_layout_set_font_description (layout, desc);
		PangoAttrList *l = pango_attr_list_new ();
		pango_layout_set_attributes (layout, l);
		pango_layout_set_font_description (layout, desc);
		pango_layout_set_text (layout, "l", -1);
		PangoLayoutIter* iter = pango_layout_get_iter (layout);
		m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		if (m_Justified)
			pango_layout_set_justify (layout, true);
		else
			pango_layout_set_alignment (layout, m_Align);
		PangoRectangle rect;
		pango_layout_get_extents (layout, NULL, &rect);
		const_cast <Text *> (this)->m_length = rect.width / PANGO_SCALE;
		const_cast <Text *> (this)->m_height = rect.height / PANGO_SCALE;
		g_object_unref (layout);
	}
	double x = m_x * theme->GetZoomFactor ();
	double y = m_y * theme->GetZoomFactor ();
	gccv::Text *text = new gccv::Text (view->GetCanvas ()->GetRoot (), x, y, this);
	text->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
	text->SetPadding (theme->GetPadding ());
	text->SetLineColor (0);
	text->SetLineOffset (view->GetCHeight ());
	text->SetAnchor (m_Anchor);
	text->SetFontDescription (desc);
	pango_font_description_free (desc);
	text->SetText (m_buf.c_str ());
	while (!m_TagList.empty ()) {
		text->InsertTextTag (m_TagList.front ());
		m_TagList.pop_front ();
	}
	m_Item = text;
}

void Text::UpdateItem ()
{
	Document *doc = static_cast <Document*> (GetDocument ());
	Theme *theme = doc->GetTheme ();
	reinterpret_cast <gccv::Text *> (m_Item)->SetPosition (m_x * theme->GetZoomFactor (), m_y * theme->GetZoomFactor ());
}

/*void Text::Add (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] != NULL)
		return;
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	if (m_ascent <= 0) {
		PangoContext* pc = pData->m_View->GetPangoContext ();
		const_cast <Text *> (this)->m_Layout = pango_layout_new (pc);
		PangoAttrList *l = pango_attr_list_new ();
		pango_layout_set_attributes (m_Layout, l);
		PangoFontDescription *desc = pango_font_description_new ();
		pango_font_description_set_family (desc, pData->m_View->GetDoc ()->GetTextFontFamily ());
		pango_font_description_set_style (desc, pData->m_View->GetDoc ()->GetTextFontStyle ());
		pango_font_description_set_variant (desc, pData->m_View->GetDoc ()->GetTextFontVariant ());
		pango_font_description_set_weight (desc, pData->m_View->GetDoc ()->GetTextFontWeight ());
		pango_font_description_set_size (desc, pData->m_View->GetDoc ()->GetTextFontSize ());
		pango_layout_set_font_description (m_Layout, desc);
		pango_font_description_free (desc);
		pango_layout_set_text (m_Layout, "l", -1);
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		const_cast <Text *> (this)->m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		pango_layout_set_text (m_Layout, m_buf.c_str (), -1);
		const_cast <Text *> (this)->m_buf.clear ();
		if (m_AttrList) {
			pango_layout_set_attributes (m_Layout, m_AttrList);
			pango_attr_list_unref (m_AttrList);
			const_cast <Text *> (this)->m_AttrList = NULL;
		}
		if (m_Justified)
			pango_layout_set_justify (m_Layout, true);
		else
			pango_layout_set_alignment (m_Layout, m_Align);
		PangoRectangle rect;
		pango_layout_get_extents (m_Layout, NULL, &rect);
		const_cast <Text *> (this)->m_length = rect.width / PANGO_SCALE;
		const_cast <Text *> (this)->m_height = rect.height / PANGO_SCALE;
	}
	double x = m_x * pTheme->GetZoomFactor ();
	switch (m_Anchor) {
	case GTK_ANCHOR_E:
		x -= m_length;
		break;
	case GTK_ANCHOR_CENTER:
		x -= m_length / 2;
		break;
	default:
		break;
	}
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type(), NULL));
	GnomeCanvasItem* item = gnome_canvas_item_new (
						group,
						gnome_canvas_rect_ext_get_type (),
						"x1", x - pTheme->GetPadding (),
						"y1", m_y * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_ascent,
						"x2", x + m_length + pTheme->GetPadding (),
						"y2", m_y * pTheme->GetZoomFactor () + m_height + pTheme->GetPadding () - m_ascent,
						"fill_color", "white",
						"outline_color", "white",
						NULL);
	g_object_set_data (G_OBJECT (group), "rect", item);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
	item = gnome_canvas_item_new (
						group,
						gnome_canvas_pango_get_type (),
						"layout", m_Layout,
						"x", x,
						"y", m_y * pTheme->GetZoomFactor () - m_ascent,
						"editing", false,
						NULL);
	g_object_set_data (G_OBJECT (group), "text", item);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_signal_connect_swapped (G_OBJECT(item), "changed", G_CALLBACK (on_text_changed), (void *) this);
	g_signal_connect_swapped (G_OBJECT (item), "sel-changed", G_CALLBACK (on_text_sel_changed), (void *) this);
	pData->Items[this] = group;
}*/

bool Text::OnChanged (bool save)
{
	Document* pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return false;
	View* pView = pDoc->GetView ();
	GtkWidget* pWidget = pView->GetWidget ();
/*	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
	GnomeCanvasGroup *group = pData->Items[this];
	if (!group) {
		pData->Items.erase (this);
		return false;
	}*/
/*	if (strlen (pango_layout_get_text (m_Layout))) {
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
	}
	PangoRectangle rect;
	pango_layout_get_extents (m_Layout, NULL, &rect);
	m_length = rect.width / PANGO_SCALE;
	m_height = rect.height / PANGO_SCALE;
	pView->Update (this);*/
	EmitSignal (OnChangedSignal);
/*	GnomeCanvasPango *PangoItem = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (group), "text"));
	unsigned CurPos = gnome_canvas_pango_get_cur_index (PangoItem);
	m_StartSel = m_EndSel = CurPos;*/
	if (save) {
		Tool* TextTool = dynamic_cast<Application*> (pDoc->GetApplication ())->GetTool ("Text");
		if (!TextTool)
			return  true;
		xmlNodePtr node = SaveSelected ();
		if (node)
			TextTool->PushNode (node);
	}
	return true;
}

/*void Text::Update (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = pData->Items[this];
	if (m_Justified)
		pango_layout_set_justify (m_Layout, true);
	else
		pango_layout_set_alignment (m_Layout, m_Align);
	double x = m_x * pTheme->GetZoomFactor ();
	switch (m_Anchor) {
	case GTK_ANCHOR_E:
		x -= m_length;
		break;
	case GTK_ANCHOR_CENTER:
		x -= m_length / 2;
		break;
	default:
		break;
	}
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "text")),
						"x", x,
						"y", m_y * pTheme->GetZoomFactor () - m_ascent,
						"width", m_length,
						"height", m_height,
						NULL);
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "rect")),
						"x1", x - pTheme->GetPadding (),
						"y1", m_y * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_ascent,
						"x2", x + m_length + pTheme->GetPadding (),
						"y2", m_y * pTheme->GetZoomFactor () + m_height + pTheme->GetPadding () - m_ascent,
						NULL);
}*/

void Text::SetSelected (int state)
{
	GOColor color;
	switch (state) {	
	case SelStateUnselected:
		color = 0;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = RGBA_WHITE;
		break;
	}
	dynamic_cast <gccv::LineItem *> (m_Item)->SetLineColor (color);
}

bool Text::OnEvent (GdkEvent *event)
{
	if ((event->type == GDK_BUTTON_PRESS) && (event->button.button == 2))
		return true;
	else
		return false;
}

void Text::Transform2D (Matrix2D& m, double x, double y)
{
	m_x += m_length / 2 - x;
	m_y += m_height / 2 - m_ascent +  - y;
	m.Transform (m_x, m_y);
	m_x -= m_length / 2 - x;
	m_y -= m_height / 2 - m_ascent - y;
}
	
double Text::GetYAlign ()
{
	return m_y - ((Document*) GetDocument ())->GetView ()->GetBaseLineOffset ();
}

bool Text::SetProperty (unsigned property, char const *value)
{
	switch (property) {
	case GCU_PROP_POS2D: {
		double x, y;
		sscanf (value, "%lg %lg", &x, &y);
		gcu::Document *doc = GetDocument ();
		if (doc) {
			x *= doc->GetScale ();
			y *= doc->GetScale ();
		}
		SetCoords (x, y);
		break;
	}
	case GCU_PROP_TEXT_MARKUP: {
		xmlDocPtr xml = xmlParseMemory (value, strlen (value));
		xmlNodePtr node = xml->children->children;
		unsigned pos = 0;
		m_buf.clear ();
		m_bLoading = true;
		while (node) {
			if (!LoadNode (node, pos, 1))
				return false;
			node = node->next;
		}
		m_bLoading = false;
		// FIXME: implement
		break;
	}
	case GCU_PROP_TEXT_TEXT:
		m_buf = value;
		break;
	case GCU_PROP_TEXT_ALIGNMENT:
		if (!strcmp (value, "right"))
				m_Anchor = gccv::AnchorLineEast;
		else if (!strcmp (value, "left"))
				m_Anchor = gccv::AnchorLineWest;
		else if (!strcmp (value, "center"))
				m_Anchor = gccv::AnchorLine;
		break;
	case GCU_PROP_TEXT_JUSTIFICATION:
		if (!strcmp (value, "right"))
				m_Align = PANGO_ALIGN_RIGHT;
		else if (!strcmp (value, "left"))
				m_Align = PANGO_ALIGN_LEFT;
		else if (!strcmp (value, "center"))
				m_Align = PANGO_ALIGN_CENTER;
		else if (!strcmp (value, "justify"))
				m_Justified = true;
		break;
	}
	return true;
}

}	//	namespace gcp
