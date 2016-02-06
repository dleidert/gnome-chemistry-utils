// -*- C++ -*-

/*
 * GChemPaint library
 * text.cc
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "text.h"
#include "widgetdata.h"
#include "view.h"
#include "document.h"
#include "application.h"
#include "fragment.h"
#include "settings.h"
#include "theme.h"
#include "tool.h"
#include "window.h"
#include <gccv/canvas.h>
#include <gccv/text.h>
#include <gcu/formula.h>
#include <gcu/objprops.h>
#include <gcu/xml-utils.h>
#include <glib/gi18n-lib.h>
#include <stdexcept>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

Text::Text ():
	TextObject (TextType),
	m_Anchor (gccv::AnchorLineWest),
	m_GlobalTag (gccv::Invalid),
	m_Interline (0.),
	m_Justification (GTK_JUSTIFY_LEFT)
{
}

Text::Text (double x, double y):
	TextObject (x, y, TextType),
	m_Anchor (gccv::AnchorLineWest),
	m_GlobalTag (gccv::Invalid),
	m_Interline (0.),
	m_Justification (GTK_JUSTIFY_LEFT)
{
}

Text::Text (gccv::Tag tag, double x, double y):
	TextObject (x, y, TextType),
	m_Anchor (gccv::AnchorLineWest),
	m_GlobalTag (tag),
	m_Interline (0.),
	m_Justification (GTK_JUSTIFY_LEFT)
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
		gccv::TextDecoration u = static_cast <gccv::UnderlineTextTag *> (m_tag)->GetUnderline ();
		char const *type = NULL;
		if (u == gccv::TextDecorationNone)
			break;
		switch (u) {
		default:
		case gccv::TextDecorationDefault:
			break;
		case gccv::TextDecorationHigh:
			type = "high"; // not really implemented
			break;
		case gccv::TextDecorationMedium:
			type = "medium"; // not really implemented
			break;
		case gccv::TextDecorationLow:
			type = "low";
			break;
		case gccv::TextDecorationDouble:
			type = "double";
			break;
		case gccv::TextDecorationSquiggle:
			type = "squiggle"; // not really implemented
			break;
		}
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("u"), NULL);
		if (u != gccv::TextDecorationDefault)
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
		WriteColor (child, static_cast <gccv::UnderlineTextTag *> (m_tag)->GetColor ());
		break;
	}
	case gccv::Overline: { // not really implemented
		gccv::TextDecoration o = static_cast <gccv::OverlineTextTag *> (m_tag)->GetOverline ();
		char const *type = NULL;
		if (o == gccv::TextDecorationNone)
			break;
		switch (o) {
		default:
		case gccv::TextDecorationDefault:
			break;
		case gccv::TextDecorationHigh:
			type = "high";
			break;
		case gccv::TextDecorationMedium:
			type = "medium";
			break;
		case gccv::TextDecorationLow:
			type = "low";
			break;
		case gccv::TextDecorationDouble:
			type = "double";
			break;
		case gccv::TextDecorationSquiggle:
			type = "squiggle";
			break;
		}
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("o"), NULL);
		if (o != gccv::TextDecorationDefault)
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
		WriteColor (child, static_cast <gccv::OverlineTextTag *> (m_tag)->GetColor ());
		break;
	}
	case gccv::Strikethrough: {
		gccv::TextDecoration s = static_cast <gccv::StrikethroughTextTag *> (m_tag)->GetStrikethrough ();
		char const *type = NULL;
		if (s == gccv::TextDecorationNone)
			break;
		switch (s) {
		default:
		case gccv::TextDecorationDefault:
			break;
		case gccv::TextDecorationHigh:
			type = "high"; // not really implemented
			break;
		case gccv::TextDecorationMedium:
			type = "medium"; // not really implemented
			break;
		case gccv::TextDecorationLow:
			type = "low";
			break;
		case gccv::TextDecorationDouble:
			type = "double";
			break;
		case gccv::TextDecorationSquiggle:
			type = "squiggle"; // not really implemented
			break;
		}
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("s"), NULL);
		if (s != gccv::TextDecorationDefault)
			xmlNewProp (child, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (type));
		WriteColor (child, static_cast <gccv::StrikethroughTextTag *> (m_tag)->GetColor ());
		break;
	}
	case gccv::Foreground: {
		GOColor color = static_cast <gccv::ForegroundTextTag *> (m_tag)->GetColor ();
		if (color == GO_COLOR_BLACK)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("fore"), NULL);
		WriteColor (child, color);
		break;
	}
	case gccv::Background: {
		GOColor color = static_cast <gccv::BackgroundTextTag *> (m_tag)->GetColor ();
		if (color == GO_COLOR_BLACK)
			break;
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("back"), NULL);
		WriteColor (child, color);
		break;
	}
	case gccv::Rise: {
		double rise = static_cast <gccv::RiseTextTag *> (m_tag)->GetRise ();
		if (rise != 0) {
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ((rise > 0)? "sup": "sub"), NULL);
			char *buf = g_strdup_printf ("%g", fabs (rise) / PANGO_SCALE);
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
	case gccv::NewLine:
		child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("br"), NULL);
		index++;
		break;
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
	case gccv::AnchorLineEast:
		xmlNewProp (node, (xmlChar const *) "anchor", (xmlChar const *) "right");
		break;
	case gccv::AnchorLine:
		xmlNewProp (node, (xmlChar const *) "anchor", (xmlChar const *) "center");
		break;
	default:
		break;
	}
	switch (m_Justification) {
	case GTK_JUSTIFY_FILL:
		xmlNewProp (node, (xmlChar const *) "justification", (xmlChar const *) "justify");
		break;
	case GTK_JUSTIFY_RIGHT:
		xmlNewProp (node, (xmlChar const *) "justification", (xmlChar const *) "right");
		break;
	case GTK_JUSTIFY_CENTER:
		xmlNewProp (node, (xmlChar const *) "justification", (xmlChar const *) "center");
		break;
	default:
		break;
	}
	if (m_GlobalTag != gccv::Invalid) {
		if (m_GlobalTag == StoichiometryTag)
			xmlNewProp (node, reinterpret_cast < xmlChar const * > ("role"), reinterpret_cast < xmlChar const * > ("stoichiometry"));
		// TODO: support other tags, such as chemistry
		xmlNodeAddContent (node, reinterpret_cast <xmlChar const *> (m_buf.c_str ()));
		return node;
	}
	if (m_Interline > 0.) {
		char *buf = g_strdup_printf ("%g", m_Interline);
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("interline"), reinterpret_cast <xmlChar *> (buf));
		g_free (buf);

	}
	unsigned i = 0;
	SaveStruct *head = NULL, *cur;
	std::list <gccv::TextTag *> const *tags;
	if (m_Item) {
		gccv::Text *t = reinterpret_cast <gccv::Text *> (m_Item);
		tags = t->GetTags ();
	} else
		tags = &m_TagList;
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

xmlNodePtr Text::SaveSelection (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "text", NULL);
	if (!node)
		return NULL;
	// get the selected text
	string selection (m_buf.substr (m_StartSel, m_EndSel - m_StartSel));
	// build the relevant tags list
	std::list <gccv::TextTag *> const *tags = reinterpret_cast <gccv::Text *> (m_Item)->GetTags ();
	std::list <gccv::TextTag *> seltags;
	std::list <gccv::TextTag *>::const_iterator i, iend = tags->end ();
	for (i = tags->begin (); i != iend; i++)
		if ((*i)->GetStartIndex () < m_EndSel && (*i)->GetEndIndex () > m_StartSel) {
			gccv::TextTag *tag = (*i)->Duplicate ();
			tag->SetStartIndex (((*i)->GetStartIndex () > m_StartSel)? (*i)->GetStartIndex () - m_StartSel : 0);
			tag->SetEndIndex (((*i)->GetEndIndex () < m_EndSel)? (*i)->GetEndIndex () - m_StartSel : m_EndSel - m_StartSel);
			seltags.push_back (tag);
		}
	// order the tags
	seltags.sort (tag_order);
	// now save the text and tags
	// first buid the tags tree
	SaveStruct *head = NULL, *cur;
	gccv::TextTagList::iterator j, jend = seltags.end ();
	for (j = seltags.begin (); j != jend; j++) {
		cur = new SaveStruct (*j, (*j)->GetStartIndex (), (*j)->GetEndIndex ());
		cur->Filter (&head);
	}
	// now, save the tree
	unsigned index = 0;
	if (head)
		head->Save (xml, node, index, selection, 0, 0, NULL, 0);
	xmlNodeAddContent (node, reinterpret_cast <xmlChar const *> (selection.c_str () + index));
	delete head;
	return (TextObject::SaveNode (xml, node))? node: NULL;
}

bool Text::Load (xmlNodePtr node)
{
	if (!TextObject::Load (node))
		return false;
	xmlChar *buf = xmlGetProp (node, (xmlChar const *) "justification");
	if (buf) {
		if (!strcmp ((char const *) buf, "justify"))
			m_Justification = GTK_JUSTIFY_FILL;
		else if (!strcmp ((char const *) buf, "right"))
			m_Justification = GTK_JUSTIFY_RIGHT;
		else if (!strcmp ((char const *) buf, "center"))
			m_Justification = GTK_JUSTIFY_CENTER;
		else
			m_Justification = GTK_JUSTIFY_LEFT;
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
	buf = xmlGetProp (node, (xmlChar const *) "interline");
	if (buf) {
		m_Interline = strtod (reinterpret_cast <char *> (buf), NULL);
		xmlFree (buf);
	}
	buf = xmlGetProp (node, (xmlChar const *) "role");
	if (buf) {
		if (!strcmp (reinterpret_cast <char *> (buf), "stoichiometry")) {
			if (StoichiometryTag == gccv::Invalid)
				StoichiometryTag = gccv::TextTag::RegisterTagType ();
			m_GlobalTag = StoichiometryTag;
			m_Anchor = gccv::AnchorSouthWest;
		}
		xmlFree (buf);
	}
	xmlNodePtr child;
	m_bLoading = true;
	child = node->children;
	m_buf.clear ();
	unsigned pos = 0;
	while (child) {
		if (!LoadNode (child, pos))
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
		text->SetJustification (m_Justification);
		text->SetInterline (m_Interline);
	}
	m_bLoading = false;
	GetDocument ()->ObjectLoaded (this);
	return true;
}

bool Text::LoadSelection (xmlNodePtr node, unsigned pos)
{
	xmlNodePtr child;
	m_bLoading = true;
	child = node->children;
	while (child) {
		if (!LoadNode (child, pos))
			return false;
		child = child->next;
	}
	gccv::Text *text = reinterpret_cast <gccv::Text *> (m_Item);
	text->SetText (m_buf.c_str ());
	while (!m_TagList.empty ()) {
		text->InsertTextTag (m_TagList.front ());
		m_TagList.pop_front ();
	}
	text->SetSelectionBounds (pos, pos);
	m_bLoading = false;
	OnChanged (true);
	return true;
}

bool Text::LoadNode (xmlNodePtr node, unsigned &pos, int cur_size)
{
	char* buf;
	gccv::TextTag *tag = NULL, *tag0 = NULL;
	unsigned start = pos;
	if (!strcmp ((const char*) node->name, "text")) {
		buf = (char*) xmlNodeGetContent (node);
		// discard when there are only ASCII spaces, this might break some old files
		char *cur = buf;
		bool new_line = false;
		while (cur && *cur) {
			if (!g_ascii_isspace (*cur))
				break;
			if (*cur == '\n')
				new_line = true;
			cur++;
		}
		if (new_line && !*cur) {
			xmlFree (buf);
			return true;
		}
		// search for '\n' because of old GChemPaint versions bug
		cur = buf;
		while (cur && *cur) {
			char *nl = strchr (cur, '\n');
			if (nl) {
				tag = new gccv::NewLineTextTag ();
				unsigned index = nl - buf + start;
				tag->SetStartIndex (index);
				tag->SetEndIndex (index + 1);
				m_TagList.push_front (tag);
				tag = NULL;
				cur = nl + 1;
			} else
				cur = NULL;
		}
		if (buf) {
			pos += strlen (buf);
			m_buf.insert (start, buf);
			xmlFree (buf);
		}
	} else if (!strcmp ((const char*) node->name, "br")) {
		tag = new gccv::NewLineTextTag ();
		m_buf.insert (pos, "\n");
		pos++;
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
		gccv::TextDecoration underline = gccv::TextDecorationDefault;
		buf = (char*) xmlGetProp(node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double"))
				underline = gccv::TextDecorationDouble;
			else if (!strcmp (buf, "low"))
				underline = gccv::TextDecorationLow;
			else if (!strcmp (buf, "medium"))
				underline = gccv::TextDecorationMedium;
			else if (!strcmp (buf, "high"))
				underline = gccv::TextDecorationHigh;
			else if (!strcmp (buf, "squiggle"))
				underline = gccv::TextDecorationSquiggle;
			xmlFree (buf);
		}
		tag = new gccv::UnderlineTextTag (underline, ReadColor (node));
	} else if (!strcmp ((const char*) node->name, "o")) {
		gccv::TextDecoration overline = gccv::TextDecorationDefault;
		buf = (char*) xmlGetProp(node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double"))
				overline = gccv::TextDecorationDouble;
			else if (!strcmp (buf, "low"))
				overline = gccv::TextDecorationLow;
			else if (!strcmp (buf, "medium"))
				overline = gccv::TextDecorationMedium;
			else if (!strcmp (buf, "high"))
				overline = gccv::TextDecorationHigh;
			else if (!strcmp (buf, "squiggle"))
				overline = gccv::TextDecorationSquiggle;
			xmlFree (buf);
		}
		tag = new gccv::OverlineTextTag (overline, ReadColor (node));
	} else if (!strcmp ((const char*) node->name, "s")) {
		gccv::TextDecoration strikethrough = gccv::TextDecorationDefault;
		buf = (char*) xmlGetProp(node, (xmlChar*) "type");
		if (buf) {
			if (!strcmp (buf, "double"))
				strikethrough = gccv::TextDecorationDouble;
			else if (!strcmp (buf, "low"))
				strikethrough = gccv::TextDecorationLow;
			else if (!strcmp (buf, "medium"))
				strikethrough = gccv::TextDecorationMedium;
			else if (!strcmp (buf, "high"))
				strikethrough = gccv::TextDecorationHigh;
			else if (!strcmp (buf, "squiggle"))
				strikethrough = gccv::TextDecorationSquiggle;
			xmlFree (buf);
		}
		tag = new gccv::StrikethroughTextTag (strikethrough, ReadColor (node));
	} else if (!strcmp ((const char*) node->name, "sub")) {
		buf = (char*) xmlGetProp (node, (xmlChar*) "height");
		if (buf) {
			int rise = -strtod (buf, NULL) * PANGO_SCALE;
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
			int rise = strtod (buf, NULL) * PANGO_SCALE;
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
	} else if (!strcmp ((const char*) node->name, "fore"))
		tag = new gccv::ForegroundTextTag (ReadColor (node));
	else if (!strcmp ((const char*) node->name, "back"))
		tag = new gccv::BackgroundTextTag (ReadColor (node));
	else
		return true;
	xmlNodePtr child = node->children;
	while (child) {
		if (!LoadNode (child, pos, cur_size))
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
	if (m_GlobalTag == gccv::Invalid) {
		pango_font_description_set_family (desc, doc->GetTextFontFamily ());
		pango_font_description_set_style (desc, doc->GetTextFontStyle ());
		pango_font_description_set_variant (desc, doc->GetTextFontVariant ());
		pango_font_description_set_weight (desc, doc->GetTextFontWeight ());
		pango_font_description_set_size (desc, doc->GetTextFontSize ());
	} else {
		pango_font_description_set_family (desc, theme->GetFontFamily ());
		pango_font_description_set_style (desc, theme->GetFontStyle ());
		pango_font_description_set_variant (desc, theme->GetFontVariant ());
		pango_font_description_set_weight (desc, theme->GetFontWeight ());
		pango_font_description_set_size (desc, theme->GetFontSize ());
	}
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
		PangoRectangle rect;
		pango_layout_get_extents (layout, NULL, &rect);
		const_cast <Text *> (this)->m_length = rect.width / PANGO_SCALE;
		const_cast <Text *> (this)->m_height = rect.height / PANGO_SCALE;
		g_object_unref (layout);
	}
	double x = m_x * theme->GetZoomFactor ();
	double y = m_y * theme->GetZoomFactor ();
	gccv::Text *text = new gccv::Text (view->GetCanvas ()->GetRoot (), x, y, this);
	text->SetFillColor (0);
	text->SetPadding (theme->GetPadding ());
	text->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
	text->SetLineOffset (view->GetCHeight ());
	text->SetAnchor (m_Anchor);
	text->SetFontDescription (desc);
	text->SetJustification (m_Justification);
	text->SetInterline (m_Interline);
	pango_font_description_free (desc);
	text->SetText (m_buf.c_str ());
	if (m_GlobalTag != gccv::Invalid && m_TagList.empty () && !m_buf.empty ()) {
		// add the global tag
		gccv::TextTag *tag = NULL;
		if (m_GlobalTag == gcp::StoichiometryTag)
			tag = new gcp::StoichiometryTextTag (static_cast < double > (theme->GetFontSize ()) / PANGO_SCALE);
		if (tag) {
			tag->SetStartIndex (0);
			tag->SetEndIndex (m_buf.length ());
			m_TagList.push_front (tag);
		}
	}
	while (!m_TagList.empty ()) {
		text->InsertTextTag (m_TagList.front ());
		m_TagList.pop_front ();
	}
	m_Item = m_TextItem = text;
}

void Text::UpdateItem ()
{
	if (!m_Item) {
		AddItem ();
		return;
	}
	Document *doc = static_cast <Document*> (GetDocument ());
	Theme *theme = doc->GetTheme ();
	reinterpret_cast <gccv::Text *> (m_Item)->SetPosition (m_x * theme->GetZoomFactor (), m_y * theme->GetZoomFactor ());
}

bool Text::OnChanged (bool save)
{
	Document* pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return false;
	m_buf = reinterpret_cast <gccv::Text *> (m_Item)->GetText ();
	EmitSignal (OnChangedSignal);
	if (save) {
		Tool* TextTool = dynamic_cast<Application*> (pDoc->GetApplication ())->GetTool ("Text");
		if (!TextTool)
			return  true;
		if (m_TextItem) {
			unsigned start, pos;
			m_TextItem->GetSelectionBounds (start, pos);
			SelectionChanged (start, pos);
		}
		xmlNodePtr node = SaveSelected ();
		if (node)
			TextTool->PushNode (node);
	}
	return true;
}

void Text::SetSelected (int state)
{
	if (!m_Item)
		return;
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
		color = GO_COLOR_WHITE;
		break;
	}
	dynamic_cast <gccv::LineItem *> (m_Item)->SetLineColor (color);
}

void Text::Transform2D (Matrix2D& m, double x, double y)
{
	m_x += m_length / 2 - x;
	m_y += m_height / 2 - m_ascent - y;
	m.Transform (m_x, m_y);
	m_x -= m_length / 2 - x;
	m_y -= m_height / 2 - m_ascent - y;
}

double Text::GetYAlign ()
{
	return m_y;
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
			if (!LoadNode (node, pos))
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
				m_Justification = GTK_JUSTIFY_RIGHT;
		else if (!strcmp (value, "left"))
				m_Justification = GTK_JUSTIFY_LEFT;
		else if (!strcmp (value, "center"))
				m_Justification = GTK_JUSTIFY_CENTER;
		else if (!strcmp (value, "justify"))
				m_Justification = GTK_JUSTIFY_FILL;
		break;
	}
	return true;
}

void Text::InterlineChanged (double interline)
{
	m_Interline = interline;
	OnChanged (!m_bLoading);
}

void Text::JustificationChanged (GtkJustification justification)
{
	m_Justification = justification;
	OnChanged (!m_bLoading);
}

std::string Text::Name ()
{
	return _("Text");
}

bool Text::GetCoords (double* x, double* y, double *z) const
{
	if (x == NULL || y == NULL || !m_Item)
		return false;
	double x1, y1;
	Theme *theme = static_cast <Document*> (GetDocument ())->GetTheme ();
	m_Item->GetBounds (*x, *y, x1, y1);
	*x = (*x + x1) / 2. / theme->GetZoomFactor ();
	*y = (*y + y1) / 2. / theme->GetZoomFactor ();
	if (z)
		*z = 0.;
	return true;
}

}	//	namespace gcp
