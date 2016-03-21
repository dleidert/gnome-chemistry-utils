// -*- C++ -*-

/*
 * GChemPaint library
 * fragment.cc
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "bond.h"
#include "document.h"
#include "fragment.h"
#include "fragment-residue.h"
#include "settings.h"
#include "text.h"
#include "theme.h"
#include "tool.h"
#include "view.h"
#include "widgetdata.h"
#include "window.h"
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/structs.h>
#include <gccv/text.h>
#include <gccv/text-tag.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/objprops.h>
#include <pango/pango-attributes.h>
#include <glib/gi18n-lib.h>
#include <list>
#include <cmath>
#include <cstring>
#include <sstream>

using namespace gcu;
using namespace std;

static int lenminus = strlen ("−");

namespace gcp {

gccv::Tag ChargeTag = gccv::Invalid, StoichiometryTag = gccv::Invalid;

ChargeTextTag::ChargeTextTag (double size):
	gccv::PositionTextTag (gccv::Superscript, size, true, (ChargeTag)? ChargeTag:(ChargeTag = gccv::TextTag::RegisterTagType ()))
{
}

ChargeTextTag::~ChargeTextTag ()
{
}

gccv::TextTag *ChargeTextTag::Restrict (gccv::TextTag *tag)
{
	if (tag->GetTag () == StoichiometryTag && tag->GetEndIndex () > GetStartIndex () && tag->GetStartIndex () < GetEndIndex ()) {
		if (*tag == *this) {
			if (GetStartIndex () > tag->GetStartIndex ())
				SetStartIndex (tag->GetStartIndex ());
			if (GetEndIndex () < tag->GetEndIndex ())
				SetEndIndex (tag->GetEndIndex ());
			tag->SetEndIndex (GetStartIndex ()); // makes tag invalid
			return NULL;
		}
		if (tag->GetEndIndex () > GetEndIndex ()) {
			if (tag->GetStartIndex () < GetStartIndex ()) {
				// split tag
				gccv::TextTag *new_tag = tag->Duplicate ();
				new_tag->SetStartIndex (GetEndIndex ());
				new_tag->SetEndIndex (tag->GetEndIndex ());
				tag->SetEndIndex (GetStartIndex ());
				return new_tag;
			}
			tag->SetStartIndex (GetEndIndex ());
			return NULL;
		} else {
			tag->SetEndIndex (GetStartIndex ());
			return NULL;
		}
	}
	return TextTag::Restrict (tag);
}

StoichiometryTextTag::StoichiometryTextTag (double size):
	gccv::PositionTextTag (gccv::Subscript, size, true, (StoichiometryTag)? StoichiometryTag:(StoichiometryTag = gccv::TextTag::RegisterTagType ()))
{
}

StoichiometryTextTag::~StoichiometryTextTag ()
{
}

gccv::TextTag *StoichiometryTextTag::Restrict (gccv::TextTag *tag)
{
	if (tag->GetTag () == ChargeTag && tag->GetEndIndex () > GetStartIndex () && tag->GetStartIndex () < GetEndIndex ()) {
		if (*tag == *this) {
			if (GetStartIndex () > tag->GetStartIndex ())
				SetStartIndex (tag->GetStartIndex ());
			if (GetEndIndex () < tag->GetEndIndex ())
				SetEndIndex (tag->GetEndIndex ());
			tag->SetEndIndex (GetStartIndex ()); // makes tag invalid
			return NULL;
		}
		if (tag->GetEndIndex () > GetEndIndex ()) {
			if (tag->GetStartIndex () < GetStartIndex ()) {
				// split tag
				gccv::TextTag *new_tag = tag->Duplicate ();
				new_tag->SetStartIndex (GetEndIndex ());
				new_tag->SetEndIndex (tag->GetEndIndex ());
				tag->SetEndIndex (GetStartIndex ());
				return new_tag;
			}
			tag->SetStartIndex (GetEndIndex ());
			return NULL;
		} else {
			tag->SetEndIndex (GetStartIndex ());
			return NULL;
		}
	}
	return TextTag::Restrict (tag);
}

// FIXME: search for unuseful things, such as m_CHeight? is it really unused?
Fragment::Fragment ():
	TextObject (FragmentType),
	m_Inversable (false),
	m_Valid (Invalid),
	m_Mode (AutoMode)
{
	m_Atom = new FragmentAtom (this, 0);
	m_BeginAtom = m_EndAtom = 0;
	m_StartSel = m_EndSel = 0;
	m_lbearing = 0;
	m_CHeight = 0.;
	SetId ("f1");
}

Fragment::Fragment (double x, double y):
	TextObject (x, y, FragmentType),
	m_Inversable (false),
	m_Valid (Invalid),
	m_Mode (AutoMode)
{
	m_Atom = new FragmentAtom (this, 0);
	m_Atom->SetCoords (x, y);
	m_BeginAtom = m_EndAtom = 0;
	m_lbearing = 0;
	m_CHeight = 0.;
	SetId ("f1");
}

Fragment::~Fragment ()
{
	if (m_Atom)
		delete m_Atom;
}

bool Fragment::OnChanged (bool save)
{
	if (m_bLoading)
		return false;
	Document* pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return false;
	int length = m_buf.length ();
	m_buf = m_TextItem->GetText ();
	if (length)
		length -= m_buf.length ();
	View* pView = pDoc->GetView ();
	unsigned CurPos = m_TextItem->GetCursorPosition ();
	AnalContent (m_StartSel, CurPos);
	m_bLoading = true;
	/*main atom management*/
	if (m_EndAtom > m_BeginAtom) {
		if (m_EndSel > CurPos && m_BeginAtom >= m_EndSel) {
			// if text was deleted before the atom, adjust its bounds
			m_BeginAtom -= m_EndSel - CurPos;
			m_EndAtom -= m_EndSel - CurPos;
		} else if (length > 0 && CurPos < m_BeginAtom + length) {
			if (m_BeginAtom >= static_cast <unsigned> (length)) {
				m_BeginAtom -= length;
				m_EndAtom -= length;
			} else {
				m_BeginAtom = m_EndAtom = 0;
			}
		}
	}
	if (m_buf.length () < m_EndAtom) { // needed if the symbol of part of it has been destroyed
		m_Atom->SetZ (0);
		m_EndAtom = m_buf.length ();
		if (m_BeginAtom > m_EndAtom)
			m_BeginAtom = m_EndAtom;
	}
	// don't take tagged glyphs into consideration
	std::list <gccv::TextTag *> const *tags = m_TextItem->GetTags ();
	std::list <gccv::TextTag *>::const_iterator tag, tag_end = tags->end ();
	unsigned start = m_BeginAtom, end = m_buf.length ();
	FragmentResidue *residue = dynamic_cast <FragmentResidue*> (m_Atom);
	Residue *r = NULL;
	char sy[Residue::MaxSymbolLength + 1];
	for (tag = tags->begin (); tag != tag_end; tag++) {
		if ((*tag)->GetStartIndex () <= start && (*tag)->GetEndIndex () > start)
			start = (*tag)->GetEndIndex ();
		else if ((*tag)->GetStartIndex () >= start && (*tag)->GetStartIndex () < end)
			end = (*tag)->GetStartIndex ();
	}
	if (!m_Atom->GetZ () || (residue != NULL && residue->GetResidue () == NULL)) {
		strncpy (sy, m_buf.c_str () + start, Residue::MaxSymbolLength);
		int i = MIN (Residue::MaxSymbolLength, end - start);
		while (i > 0) {
			sy[i] = 0;
			r = (Residue *) Residue::GetResidue (sy, NULL);
			if (r || Element::Z (sy) > 0)
				break;
			i--;
		}
		if (r) {
			CurPos = start + strlen (sy);
			m_BeginAtom = start;
			m_EndAtom = CurPos;
			if (residue)
				residue->SetResidue (r, sy);
			else {
				map < gcu::Bondable *, gcu::Bond * >::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentResidue (this, sy);
				m_Atom->SetId ((char *) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				double x, y;
				pOldAtom->GetCoords (&x, &y);
				m_Atom->SetCoords (x, y);
				delete pOldAtom;
				AddChild (m_Atom);
			}
		} else {
			CurPos = end;
			int Z = GetElementAtPos (m_StartSel, CurPos);
			if (!Z && m_StartSel > m_BeginAtom)
				Z = GetElementAtPos (m_StartSel = m_BeginAtom, CurPos);
			if (Z) {
				if (residue) {
					map < gcu::Bondable *, gcu::Bond * >::iterator i;
					Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
					Atom *pOldAtom = m_Atom;
					m_Atom = new FragmentAtom (this, Z);
					m_Atom->SetId ((char *) pOldAtom->GetId ());
					if (pBond) {
						pBond->ReplaceAtom (pOldAtom, m_Atom);
						m_Atom->AddBond (pBond);
					}
					double x, y;
					pOldAtom->GetCoords (&x, &y);
					m_Atom->SetCoords (x, y);
					delete pOldAtom;
					AddChild (m_Atom);
				} else
					m_Atom->SetZ (Z);
				m_BeginAtom = m_StartSel;
				m_EndAtom = CurPos;
			}
		}
	} else if (m_EndSel <= m_BeginAtom) {
		int delta = CurPos - m_EndSel;
		if (delta > 0) { // negative values have already been dealt with
			m_BeginAtom += delta;
			m_EndAtom += delta;
		}
	} else if ((m_EndAtom <= m_EndSel && m_EndAtom >= m_StartSel) ||
		(m_BeginAtom <= m_EndSel && m_BeginAtom >= m_StartSel) ||
		(m_BeginAtom + Residue::MaxSymbolLength >= CurPos)) {
		if (m_BeginAtom > start)
			m_BeginAtom = start;
		unsigned maxlength = MIN (Residue::MaxSymbolLength, end - m_BeginAtom);
		if (m_EndAtom > end)
			m_EndAtom = end;
		else if (m_EndAtom < m_BeginAtom + maxlength)
			m_EndAtom = m_BeginAtom + maxlength;
		strncpy (sy, m_buf.c_str () + m_BeginAtom, maxlength);
		unsigned i = maxlength;
		while (i > 0) {
			sy[i] = 0;
			r = (Residue *) Residue::GetResidue (sy, NULL);
			if (r || Element::Z (sy))
				break;
			i--;
		}
		if (r) {
			m_EndAtom = m_BeginAtom + strlen (sy);
			if (residue)
				residue->SetResidue (r, sy);
			else {
				map < gcu::Bondable *, gcu::Bond * >::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentResidue (this, sy);
				m_Atom->SetId ((char *) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				double x, y;
				pOldAtom->GetCoords (&x, &y);
				m_Atom->SetCoords (x, y);
				delete pOldAtom;
				AddChild (m_Atom);
			}
		} else {
			int Z = GetElementAtPos (m_BeginAtom, m_EndAtom);
			if (residue) {
				map < gcu::Bondable *, gcu::Bond * >::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentAtom (this, Z);
				m_Atom->SetId ((char *) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				double x, y;
				pOldAtom->GetCoords (&x, &y);
				m_Atom->SetCoords (x, y);
				delete pOldAtom;
				AddChild (m_Atom);
			} else
				m_Atom->SetZ (Z);
			if (!Z)
				m_EndAtom = CurPos;
		}
	}
	m_Inversable = (m_BeginAtom == 0 || m_EndAtom == m_buf.size ()) && ((m_EndAtom - m_BeginAtom) < m_buf.length ());
	gccv::Rect rect;
	m_TextItem->GetPositionAtIndex (m_BeginAtom, rect);
	m_lbearing = rect.x0;
	m_TextItem->GetPositionAtIndex (m_EndAtom, rect);
	m_lbearing += rect.x0;
	m_lbearing /=  2;
	pView->Update (this);
	m_bLoading = false;
	m_Valid = Invalid;
	Window *pWin = static_cast < Window * > (pDoc->GetWindow ());
	if (m_Atom->GetZ () || ((m_buf.length () == 0) && (m_Atom->GetBondsNumber () == 0))) {
		if (!pDoc->GetReadOnly ()) {
			pWin->ActivateActionWidget ("/MainMenu/FileMenu/Save", true);
			pWin->ActivateActionWidget ("/MainToolbar/Save", true);
		}
		pWin->ActivateActionWidget ("/MainMenu/FileMenu/SaveAs", true);
		pWin->ActivateActionWidget ("/MainMenu/FileMenu/Print", true);
	} else {
		pWin->ActivateActionWidget ("/MainMenu/FileMenu/Save", false);
		pWin->ActivateActionWidget ("/MainMenu/FileMenu/SaveAs", false);
		pWin->ActivateActionWidget ("/MainMenu/FileMenu/Print", false);
		pWin->ActivateActionWidget ("/MainToolbar/Save", false);
	}
	EmitSignal (OnChangedSignal);
	if (m_buf.length () == 0) {
		m_BeginAtom = m_EndAtom = 0;
	}
	if (save) {
		Tool* FragmentTool = dynamic_cast<Application*> (pDoc->GetApplication ())->GetTool ("Fragment");
		if (!FragmentTool)
			return  true;
		if (m_TextItem) {
			unsigned start, pos;
			m_TextItem->GetSelectionBounds (start, pos);
			SelectionChanged (start, pos);
		}
		xmlNodePtr node = SaveSelected ();
		if (node)
			FragmentTool->PushNode (node);
	}
	return true;
}

void Fragment::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	PangoFontDescription *desc = view->GetPangoFontDesc ();
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
		g_object_unref (layout);
	}
	double x = m_x * theme->GetZoomFactor ();
	double y = m_y * theme->GetZoomFactor ();
	gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), x, y, this);
	m_TextItem = new gccv::Text (group, 0., 0., this);
	m_TextItem->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
	m_TextItem->SetPadding (theme->GetPadding ());
	m_TextItem->SetFillColor (0);
	m_TextItem->SetLineColor (0);
	m_TextItem->SetLineOffset (view->GetCHeight ());
	m_TextItem->SetAnchor (gccv::AnchorLineWest);
	m_TextItem->SetFontDescription (desc);
	m_TextItem->SetText (m_buf);
	while (!m_TagList.empty ()) {
		m_TextItem->InsertTextTag (m_TagList.front (), false);
		m_TagList.pop_front ();
	}
	m_TextItem->RebuildAttributes ();
	if (m_buf.length () > 0) {
		gccv::Rect rect;
		m_TextItem->GetPositionAtIndex (m_BeginAtom, rect);
		m_lbearing = rect.x0;
		m_TextItem->GetPositionAtIndex (m_EndAtom, rect);
		m_lbearing += rect.x0;
		m_lbearing /= 2.;
		m_TextItem->Move (-m_lbearing, 0.);
	}
	m_Atom->DoBuildSymbolGeometry (view);
	m_Item = group;
	int charge = m_Atom->GetCharge ();
	if (charge) {
		double x, y, Angle, Dist;
		unsigned char Pos = m_Atom->Atom::GetChargePosition (&Angle, &Dist);
		gccv::Anchor anchor = const_cast <Fragment *> (this)->GetChargePosition (m_Atom, Pos, 0., x, y);
		if (Dist != 0.) {
			anchor = gccv::AnchorCenter;
			x = Dist * cos (Angle);
			y = Dist * sin (Angle);
		}
		x -= m_x;
		x *= theme->GetZoomFactor ();
		y -= m_y;
		y *= theme->GetZoomFactor ();
		char const *glyph = (charge > 0)? "\xE2\x8a\x95": "\xE2\x8a\x96";
		gccv::Text *text = new gccv::Text (group, x, y, this);
		text->SetFillColor (0);
		text->SetPadding (theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetAnchor (anchor);
		text->SetFontDescription (view->GetPangoSmallFontDesc ());
		text->SetText (glyph);
		m_Atom->SetChargeItem (text);
	} else
		m_Atom->SetChargeItem (NULL);
	// update the size
	gccv::Rect rect;
	m_TextItem->GetBounds (&rect, NULL);
	m_length = rect.x1 - rect.x0;
	m_height = rect.y1 - rect.y0;
}

void Fragment::UpdateItem ()
{
	if (!m_TextItem)
		return;
	if (Update ()) {
		delete m_TextItem;
		m_TextItem = NULL;
		delete m_Item;
		m_Item = NULL;
		AddItem ();
		return;
	}
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	gccv::Group *group = static_cast <gccv::Group *> (m_Item);
	group->SetPosition (m_x * theme->GetZoomFactor (), m_y * theme->GetZoomFactor ());
	m_TextItem->SetPosition (-m_lbearing, 0);
	int charge = m_Atom->GetCharge ();
	if (charge) {
		double x, y, Angle, Dist;
		unsigned char Pos = m_Atom->Atom::GetChargePosition (&Angle, &Dist);
		gccv::Anchor anchor = const_cast <Fragment *> (this)->GetChargePosition (m_Atom, Pos, 0., x, y);
		if (Dist != 0.) {
			anchor = gccv::AnchorCenter;
			x = Dist * cos (Angle);
			y = Dist * sin (Angle);
		}
		x -= m_x;
		x *= theme->GetZoomFactor ();
		y -= m_y;
		y *= theme->GetZoomFactor ();
		if (m_Atom->GetChargeItem ()) {
			gccv::Text *text = static_cast <gccv::Text *> (m_Atom->GetChargeItem ());
			text->SetPosition (x, y);
			text->SetAnchor (anchor);
		} else {
			char const *glyph = (charge > 0)? "\xE2\x8a\x95": "\xE2\x8a\x96";
			gccv::Text *text = new gccv::Text (group, x, y, NULL);
			text->SetFillColor (0);
			text->SetPadding (theme->GetPadding ());
			text->SetLineColor (0);
			text->SetLineWidth (0.);
			text->SetAnchor (anchor);
			text->SetFontDescription (view->GetPangoSmallFontDesc ());
			text->SetText (glyph);
			m_Atom->SetChargeItem (text);
		}
	} else if (m_Atom->GetChargeItem ()) {
		delete m_Atom->GetChargeItem ();
		m_Atom->SetChargeItem (NULL);
	}
	const_cast <FragmentAtom *> (m_Atom)->DoBuildSymbolGeometry (view);
}

void Fragment::SetSelected (int state)
{
	GOColor color, othercolor = 0;
	switch (state) {
	case SelStateUnselected:
		color = GO_COLOR_BLACK;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		othercolor = AddColor;
		color = GO_COLOR_BLACK;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = GO_COLOR_BLACK;
		break;
	}
	gccv::Group *group = static_cast <gccv::Group *> (m_Item);
	std::list<gccv::Item *>::iterator it;
	gccv::Item *item = group->GetFirstChild (it);
	while (item) {
		gccv::Text *text;
		if ((text = dynamic_cast <gccv::Text *> (item))) {
			text->SetColor (color);
			text->SetLineColor (othercolor);
		} else
			static_cast <gccv::LineItem *> (item)->SetLineColor (color);
		item = group->GetNextChild (it);
	}
}

xmlNodePtr Fragment::Save (xmlDocPtr xml) const
{
	if (m_RealSave && !const_cast <Fragment *> (this)->Validate ())
		return NULL;
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "fragment", NULL);
	if (m_buf.length ()) {
		if (!m_Atom->GetBondsNumber () || m_Atom->GetZ ()) {
			if (!node)
				return NULL;
			if (!SavePortion (xml, node, 0, m_BeginAtom)) {
				xmlFreeNode (node);
				return NULL;
			}
			if (m_Atom->GetZ  ()) {
				xmlNodePtr child = m_Atom->Save(xml);
				if (!child) {
					xmlFreeNode(node);
					return NULL;
				}
				xmlAddChild(node, child);
			}
			if (!SavePortion(xml, node, m_EndAtom, m_buf.length ())) {
				xmlFreeNode(node);
				return NULL;
			}
		}
	}
	return (SaveNode (xml, node))? node: NULL;
}

bool Fragment::SavePortion (xmlDocPtr xml, xmlNodePtr node, unsigned start, unsigned end) const
{
	std::list <gccv::TextTag *> const *tags;
	if (m_Item)
		tags = m_TextItem->GetTags ();
	else
		tags = &m_TagList;
	std::list <gccv::TextTag *>::const_iterator j, jend = tags->end ();
	xmlNodePtr child = NULL;
	char *err;
	int charge;
	string content;
	for (j = tags->begin (); j != jend; j++) {
		if (end <= (*j)->GetStartIndex () || start >= (*j)->GetEndIndex ())
			continue;
		if (start < (*j)->GetStartIndex ())
			xmlNodeAddContentLen (node, reinterpret_cast <xmlChar const *> (m_buf.c_str () + start), (*j)->GetStartIndex () - start);
		gccv::Tag tag = (*j)->GetTag ();
		if (tag == gccv::Position) {
			bool stacked;
			double size;
			gccv::TextPosition pos = static_cast <gccv::PositionTextTag *> (*j)->GetPosition (stacked, size);
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
			xmlNodeAddContentLen ((child)? child: node, reinterpret_cast <xmlChar const *> (m_buf.c_str () + (*j)->GetStartIndex ()), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
		} else if (tag == ChargeTag) {
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("charge"), NULL);
			charge = strtol (m_buf.c_str () + (*j)->GetStartIndex (), &err, 10);
			if (charge == 0 && m_buf[(*j)->GetStartIndex ()] != '0') {
				if (*err == '+' && (*j)->GetEndIndex () == err - m_buf.c_str () + 1)
					xmlNewProp (child, reinterpret_cast <xmlChar const *> ("value"), reinterpret_cast <xmlChar const *> ("1"));
				else if (!strncmp (err, "−", lenminus) && (*j)->GetEndIndex () == err - m_buf.c_str () + lenminus)
					xmlNewProp (child, reinterpret_cast <xmlChar const *> ("value"), reinterpret_cast <xmlChar const *> ("-1"));
				else
					xmlNodeAddContentLen (child, reinterpret_cast <xmlChar const *> (m_buf.c_str () + (*j)->GetStartIndex ()), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
			} else {
				bool known = (*err == '+' && (*j)->GetEndIndex () == err - m_buf.c_str () + 1);
				if (!known && !strncmp (err, "−", lenminus)/* && (*j)->GetEndIndex () == err - m_buf.c_str () + lenminus*/) {
					charge = -charge;
					known = true;
				}
				if (known) {
					char *buf = g_strdup_printf ("%d", charge);
					xmlNewProp (child, reinterpret_cast <xmlChar const *> ("value"), reinterpret_cast <xmlChar const *> (buf));
					g_free (buf);
				} else
					xmlNodeAddContentLen (child, reinterpret_cast <xmlChar const *> (m_buf.c_str () + (*j)->GetStartIndex ()), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
			}
		} else if (tag == StoichiometryTag) {
			child = xmlNewDocNode (xml, NULL, reinterpret_cast <xmlChar const *> ("stoichiometry"), NULL);
			content = m_buf.substr ((*j)->GetStartIndex (), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
			// using the charge variable
			charge = strtol (content.c_str (), &err, 10);
			if (charge <= 0 || (err && *err))
				xmlNodeAddContentLen (child, reinterpret_cast <xmlChar const *> (m_buf.c_str () + (*j)->GetStartIndex ()), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
			else {
				char *buf = g_strdup_printf ("%d", charge);
				xmlNewProp (child, reinterpret_cast <xmlChar const *> ("value"), reinterpret_cast <xmlChar const *> (buf));
				g_free (buf);
			}
		} else {
			xmlNodeAddContentLen (node, reinterpret_cast <xmlChar const *> (m_buf.c_str () + (*j)->GetStartIndex ()), (*j)->GetEndIndex () - (*j)->GetStartIndex ());
			child = NULL;
		}
		if (child)
			xmlAddChild (node, child);
		start = (*j)->GetEndIndex ();
	}
	if (start < end)
		xmlNodeAddContentLen (node, reinterpret_cast <xmlChar const *> (m_buf.c_str () + start), end - start);
	return true;
}

xmlNodePtr Fragment::SaveSelection (xmlDocPtr xml) const
{
	xmlNodePtr node = xmlNewDocNode (xml, NULL, (xmlChar*) "fragment", NULL);
	if (!node)
		return NULL;
	SavePortion (xml, node, m_StartSel, m_EndSel);
	return (TextObject::SaveNode (xml, node))? node: NULL;
}

bool Fragment::Load (xmlNodePtr node)
{
	Document* pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	if (!TextObject::Load (node))
		return false;
	m_bLoading = true;
	m_buf.clear (); // just in case
	xmlNodePtr child = node->children;
	char* tmp;
	double size = (double) pTheme->GetFontSize () / PANGO_SCALE;
	gccv::TextTag *tag;
	m_Atom->SetId ("a0"); // avoids confusion with another atom while loading
	while (child) {
		if (!strcmp ((const char*) child->name, "text")) {
			tmp = (char*) xmlNodeGetContent (child);
			if (!strchr (tmp, '\n'))
				m_buf += tmp;
			xmlFree (tmp);
		} else if (!strcmp ((const char*) child->name, "atom")) {
			AddChild (m_Atom);
			if (!m_Atom->Load (child))
				return false;
			m_BeginAtom = m_buf.length ();
			m_buf += m_Atom->GetSymbol();
			m_Atom->SetCoords (m_x, m_y);
			m_EndAtom = m_buf.length ();
		} else if (!strcmp ((const char*) child->name, "residue")) {
			// replace the atom by a residue
			map < gcu::Bondable *, gcu::Bond * >::iterator i;
			Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
			Atom *pOldAtom = m_Atom;
			m_Atom = new FragmentResidue (this, NULL);
			if (pBond) {
				pBond->ReplaceAtom (pOldAtom, m_Atom);
				m_Atom->AddBond (pBond);
			}
			delete pOldAtom;
			AddChild (m_Atom);
			if (!m_Atom->Load (child))
				return false;
			m_BeginAtom = m_buf.length ();
			m_buf += m_Atom->GetSymbol ();
			m_Atom->SetCoords (m_x, m_y);
			m_EndAtom = m_buf.length ();
		} else if (!strcmp ((const char*) child->name, "charge")) {
			int start = m_buf.length ();
			tmp = (char*) xmlGetProp (child, (xmlChar*) "value");
			if (tmp) {
				int charge = atoi (tmp);
				xmlFree (tmp);
				if (abs(charge) > 1)
					tmp = g_strdup_printf ("%d%s", abs (charge), (charge > 0)? "+": "−");
				else if (charge == 1)
					tmp = g_strdup ("+");
				else if (charge == -1)
					tmp = g_strdup ("−");
				else
					tmp = g_strdup ("");//should not occur!
				m_buf += tmp;
				g_free (tmp);
			} else { // allow things such as "z+" or "δ−"
				tmp = reinterpret_cast <char *> (xmlNodeGetContent (child));
				m_buf += tmp;
				xmlFree (tmp);
			}
			tag = new ChargeTextTag (size);
			tag->SetStartIndex (start);
			tag->SetEndIndex (m_buf.length ());
			m_TagList.push_back (tag);
		} else if (!strcmp ((const char*) child->name, "stoichiometry")) {
			int start = m_buf.length ();
			tmp = (char*) xmlGetProp (child, (xmlChar*) "value");
			if (tmp) {
				int nb = atoi (tmp);
				xmlFree (tmp);
				if (nb <=0)
					return false; // negative number is not allowed
				tmp = g_strdup_printf ("%u", nb);
				m_buf += tmp;
				g_free (tmp);
			} else { // allow things such as "n"
				tmp = reinterpret_cast <char *> (xmlNodeGetContent (child));
				m_buf += tmp;
				xmlFree (tmp);
			}
			tag = new StoichiometryTextTag (size);
			tag->SetStartIndex (start);
			tag->SetEndIndex (m_buf.length ());
			m_TagList.push_back (tag);
		} else if (!strcmp ((const char*) child->name, "sub")) {
			int start = m_buf.length ();
			tmp = reinterpret_cast <char *> (xmlNodeGetContent (child));
			m_buf += tmp;
			xmlFree (tmp);
			tag = new gccv::PositionTextTag (gccv::Subscript, (double) pTheme->GetFontSize () / PANGO_SCALE);
			tag->SetStartIndex (start);
			tag->SetEndIndex (m_buf.length ());
			m_TagList.push_back (tag);
		} else if (!strcmp ((const char*) child->name, "sup")) {
			int start = m_buf.length ();
			tmp = reinterpret_cast <char *> (xmlNodeGetContent (child));
			m_buf += tmp;
			xmlFree (tmp);
			tag = new gccv::PositionTextTag (gccv::Superscript, (double) pTheme->GetFontSize () / PANGO_SCALE);
			tag->SetStartIndex (start);
			tag->SetEndIndex (m_buf.length ());
			m_TagList.push_back (tag);
		}
		child = child->next;
	}
	if (!m_TextItem) {
		if ( static_cast <Document *> (GetDocument ())->GetSoftwareVersion () < 11000) {
			// analyse text between tags to support previous versions
			list <gccv::TextTag *> tags;
			gccv::TextTagList::iterator i, iend = m_TagList.end ();
			unsigned start = 0, end;
			for (i = m_TagList.begin (); i != iend; i++)
				tags.push_back (*i);
			list <gccv::TextTag *>::iterator j, jend = tags.end ();
			for (j = tags.begin (); j != jend; j++) {
				end = (*j)->GetStartIndex ();
				if (end > start)
					AnalContent (start, end);
				start = (*j)->GetEndIndex ();
			}
			end = m_buf.length ();
			if (end > start)
				AnalContent (start, end);
		}
	} else {
		m_TextItem->SetText (m_buf);
		while (!m_TagList.empty ()) {
			m_TextItem->InsertTextTag (m_TagList.front ());
			m_TagList.pop_front ();
		}
		m_TextItem->RebuildAttributes ();
	}
	m_Inversable = (m_BeginAtom == 0 || m_EndAtom == m_buf.size ()) && ((m_EndAtom - m_BeginAtom) < m_buf.length ());
	m_bLoading = false;
	pDoc->ObjectLoaded (this);
	pDoc->ObjectLoaded (m_Atom);
	return true;
}

void Fragment::AnalContent ()
{
	if (!m_Atom->GetParent ())
		AddChild (m_Atom);
	unsigned end = m_buf.length ();
	AnalContent (0, end);
}

typedef struct
{
	unsigned index, end;
	bool result;
} ChargeFindStruct;

void Fragment::AnalContent (unsigned start, unsigned &end)
{
	Document* pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return;
	Theme *pTheme = pDoc->GetTheme ();
	char const *text = m_buf.c_str ();
	list <gccv::TextTag *> const *tags = (m_TextItem)? m_TextItem->GetTags (): &m_TagList;
	list <gccv::TextTag *>::const_iterator i, iend;
	bool Charge = false, Stoich = false;
	gccv::TextTag *tag = NULL, *new_tag = NULL;
	char c;
	unsigned next;
	next = start;
	double size = (double) pTheme->GetFontSize () / PANGO_SCALE;
	while (start < end) {
		iend = tags->end ();
		for (i = tags->begin (); i != iend; i++){
			if ((*i)->GetStartIndex () < start && (*i)->GetEndIndex () >= start) {
				if ((*i)->GetTag () == ChargeTag) {
					Charge = true;
					tag = *i;
					break;
				}
				if ((*i)->GetTag () == StoichiometryTag) {
					tag =*i;
					Stoich = true;
					break;
				}
			}}
		if (tag && start < tag->GetEndIndex ())
			c = *g_utf8_find_prev_char (text, text + tag->GetEndIndex ());
		else
			c = text[start];
		if ((c >= '0') && (c <= '9') && (m_Mode == AutoMode)) {
			next = start + 1; // a figure is a one byte character
			// add new tag
			if (!Charge) {
				if (!Stoich) {
					new_tag = new StoichiometryTextTag (size);
					new_tag->SetStartIndex (start);
					new_tag->SetEndIndex (next);
					Stoich = true;
				} else
					tag->SetEndIndex (next);
			} else if (start == tag->GetEndIndex ()) {
				string repl (1, c);
				char *buf = g_utf8_find_prev_char (text, text + start);
				repl.append (buf, text + start - buf);
				new_tag = new ChargeTextTag (size);
				new_tag->SetStartIndex (tag->GetStartIndex ());
				new_tag->SetEndIndex (next);
				if (m_TextItem) {
					m_TextItem->ReplaceText (repl, buf - text, text + start + 1 - buf);
					m_buf = m_TextItem->GetText ();
				} else
					m_buf.replace (buf - text, repl.length (), repl.c_str ());
				text = m_buf.c_str ();
			} else
				tag->SetEndIndex (tag->GetEndIndex () + 1);
			if (new_tag) {
				if (m_TextItem)
					m_TextItem->InsertTextTag (new_tag);
				else
					m_TagList.push_back (new_tag);
				tag = new_tag;
				new_tag = NULL;
				Stoich = true;
			}
			if (start < tag->GetEndIndex ()) {
				start = tag->GetEndIndex ();
				continue;
			}
		} else if ((c == '+') || (c == '-') || !strncmp (text + start, "−", lenminus)) {
			if (!m_bLoading && (m_Mode == AutoMode)) {
				//do not allow both local and global charges
				if (m_Atom->GetCharge ())
					m_Atom->SetCharge (0);
				next = start + 1;
				if (!Charge) {
					if (c == '-') {
						string sign = "−";
						m_TextItem->ReplaceText (sign, start, 1);
						m_buf = m_TextItem->GetText ();
						text = m_buf.c_str ();
						next = start + lenminus;
						if (m_BeginAtom > start) {
							m_BeginAtom += lenminus - 1;
							m_EndAtom += lenminus - 1;
						}
					}
					new_tag = new ChargeTextTag (size);
					new_tag->SetStartIndex (start);
					new_tag->SetEndIndex (next);
					if (Stoich) {
						tag->SetEndIndex (start);
						tag = NULL;
						Stoich = false;
					}
				} else {
					// old charge is tag content minus the last character (+ or - just typed)
					string old_charge (m_buf, tag->GetStartIndex (), tag->GetEndIndex () - tag->GetStartIndex ());
					char *nextch = NULL;
					int charge = strtol (old_charge.c_str (), &nextch, 10);
					if (charge == 0)
						charge = 1;
					if (nextch && !strncmp (nextch, "−", lenminus))
						charge = -charge;
					if (*(text + tag->GetEndIndex ()) == '+')
						charge++;
					else
						charge--;
					if (charge == 0) {
						old_charge.clear ();
						end -= tag->GetEndIndex () - tag->GetStartIndex () + 1;
						next = tag->GetStartIndex ();
						if (m_BeginAtom > start) {
							m_BeginAtom -= tag->GetEndIndex () - tag->GetStartIndex ();
							m_EndAtom -= tag->GetEndIndex () - tag->GetStartIndex ();
						}
						m_TextItem->ReplaceText (old_charge, tag->GetStartIndex (), tag->GetEndIndex () - tag->GetStartIndex () + 1);
						m_buf = m_TextItem->GetText ();
						text = m_buf.c_str ();
						tag = NULL;
						Charge = false;
					} else {
						nextch = (abs (charge) > 1)? g_strdup_printf ("%d", abs (charge)): g_strdup ("");
						old_charge = string (nextch) + ((charge > 0)? "+": "−");
						end -= tag->GetEndIndex () - tag->GetStartIndex () - old_charge.length () + 1;
						start = tag->GetStartIndex ();
						next = start + old_charge.length ();
						if (m_BeginAtom > start) {
							m_BeginAtom -= tag->GetEndIndex () - tag->GetStartIndex () - old_charge.length ();
							m_EndAtom -= tag->GetEndIndex () - tag->GetStartIndex () - old_charge.length ();
						}
						m_TextItem->ReplaceText (old_charge, tag->GetStartIndex (), tag->GetEndIndex () - tag->GetStartIndex () + 1);
						m_buf = m_TextItem->GetText ();
						text = m_buf.c_str ();
						g_free (nextch);
						new_tag = new ChargeTextTag (size);
						new_tag->SetStartIndex (start);
						new_tag->SetEndIndex (next);
					}
				}
				if (new_tag) {
					if (m_TextItem)
						m_TextItem->InsertTextTag (new_tag);
					else
						m_TagList.push_back (new_tag);
					tag = new_tag;
					new_tag = NULL;
					Charge = true;
				}
				start = next;
				continue;
			} else if (c == '-' && m_Mode == ChargeMode) {
				string minus = "−";
				m_TextItem->ReplaceText (minus, start, 1);
				next = start + lenminus;
			}
		} else if (m_Mode == AutoMode) {
			Charge = false;
			Stoich = false;
			if (tag) {
				tag->SetEndIndex (start);
				tag = NULL;
			}
			if (c == '\'' || c == '"') {
				unsigned nb = 0;
				int l;
				if (start > 0) {
					next = g_utf8_find_prev_char (m_buf.c_str (), m_buf.c_str () + start) - m_buf.c_str ();
					l = start - next;
					if (!strncmp (m_buf.c_str () + next, "′", l))
						nb = 1;
					else if (!strncmp (m_buf.c_str () + next, "″", l))
						nb = 2;
					else if (!strncmp (m_buf.c_str () + next, "‴", l))
						nb = 3;
					else if (!strncmp (m_buf.c_str () + next, "⁗", l) || m_buf[next] == '\'' || m_buf[next] == '"')
						nb = 4;
					else
						next = start;
				} else {
					next = start;
					l = 0;
				}
				nb += (c == '"')? 2: 1;
				if (nb > 4)
					break;
				l++;
				string glyph;
				switch (nb) {
				case 1:
					glyph = "′";
					break;
				case 2:
					glyph = "″";
					break;
				case 3:
					glyph = "‴";
					break;
				case 4:
					glyph = "⁗";
					break;
				default: // should not happen
					break;
				}
				if (m_TextItem) {
					m_TextItem->ReplaceText (glyph, next, l);
					m_buf = m_TextItem->GetText ();
					text = m_buf.c_str ();
				} else
					m_buf.replace (next, l, glyph);
				end += glyph.length () - l;
				start = next;
			}
		}
		start = g_utf8_find_next_char (m_buf.c_str () + start, NULL) - m_buf.c_str ();
	}
	if (m_TextItem)
		m_TextItem->RebuildAttributes ();
}

/*!
Must return NULL if active tool is FragmentTool because this tool needs a fragment, not an atom
TODO: use x and y to figure the best atom in the fragment
*/
Object* Fragment::GetAtomAt (double x, double y, G_GNUC_UNUSED double z)
{
	Document* pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	Application* pApp = pDoc->GetApplication ();
	if (pApp->GetActiveTool () == pApp->GetTool ("Fragment"))
		return NULL;
	if (m_Atom->GetBondsNumber () || m_Atom->GetCharge ())
		return m_Atom;
	double x0, y0;
	x0 = (x - m_x) * pTheme->GetZoomFactor () + m_lbearing;
	y0 = (y - m_y) * pTheme->GetZoomFactor () + m_ascent;
	if ((x0 < 0.) || (x0 > m_length) || (y0 < 0.) || (y0 > m_height))
		return NULL;
	unsigned index, trailing;
	int cur;
	index = m_TextItem->GetIndexAt (x0, y0);
	if (index > 0)
		index--;
	char c = m_buf[index];
	cur = index;
	while ((c >= 'a') && (c <= 'z') && cur >= 0) {
		cur--;
		c = m_buf[cur];
	}
	if (index - cur > Residue::MaxSymbolLength)
		cur = index - Residue::MaxSymbolLength;
	if (cur < 0)
		cur = 0;
	// first search for residues
	FragmentResidue *residue = dynamic_cast <FragmentResidue*> (m_Atom);
	Residue *r = NULL;
	char sy[Residue::MaxSymbolLength + 1];
	strncpy (sy, m_buf.c_str () + cur, Residue::MaxSymbolLength);
	int i = Residue::MaxSymbolLength;
	while (i > 0) {
		sy[i] = 0;
		r = (Residue *) Residue::GetResidue (sy, NULL);
		if (r || Element::Z (sy) > 0)
			break;
		i--;
	}
	if (r) {
		m_BeginAtom = cur;
		m_EndAtom = cur + strlen (sy);
		if (residue)
			residue->SetResidue (r, sy);
		else {
			map < gcu::Bondable *, gcu::Bond * >::iterator i;
			Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
			Atom *pOldAtom = m_Atom;
			m_Atom = NULL;
			m_Atom = new FragmentResidue (this, sy);
			m_Atom->SetId ((char *) pOldAtom->GetId ());
			if (pBond) {
				pBond->ReplaceAtom (pOldAtom, m_Atom);
				m_Atom->AddBond (pBond);
			}
			delete pOldAtom;
			AddChild (m_Atom);
		}
		m_x -= m_lbearing / pTheme->GetZoomFactor ();
		gccv::Rect rect;
		m_TextItem->GetPositionAtIndex (index, rect);
		m_lbearing = rect.x0;
		m_TextItem->GetPositionAtIndex (index + i, rect);
		m_lbearing += rect.x0;
		m_lbearing /=  2;
		m_x += m_lbearing / pTheme->GetZoomFactor ();
		m_Atom->SetCoords(m_x, m_y);
		return m_Atom;
	}
	if (index - cur > 2)
		index -= 2;
	else
		index = cur;
	c = m_buf[index];
	int Z = GetElementAtPos((unsigned) index, (unsigned&) trailing);
	if (!Z)
		return NULL;
	m_bLoading = true;
	if (residue) {
		map < gcu::Bondable *, gcu::Bond * >::iterator i;
		Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
		Atom *pOldAtom = m_Atom;
		m_Atom = NULL;
		m_Atom = new FragmentAtom (this, Z);
		m_Atom->SetId ((char *) pOldAtom->GetId ());
		if (pBond) {
			pBond->ReplaceAtom (pOldAtom, m_Atom);
			m_Atom->AddBond (pBond);
		}
		delete pOldAtom;
		AddChild (m_Atom);
	}
	if (m_Atom) {
		m_Atom->SetZ (Z);
		m_bLoading = false;
		m_BeginAtom = index;
		m_EndAtom = trailing;
		m_x -= m_lbearing / pTheme->GetZoomFactor () ;
		gccv::Rect rect;
		m_TextItem->GetPositionAtIndex (index, rect);
		m_lbearing = rect.x0;
		m_TextItem->GetPositionAtIndex (trailing, rect);
		m_lbearing += rect.x0;
		m_lbearing /=  2;
		m_x += m_lbearing / pTheme->GetZoomFactor ();
		m_Atom->SetCoords(m_x, m_y);
	}

	return m_Atom;
}

void Fragment::Move (double x, double y, double z)
{
	TextObject::Move (x, y, z);
	m_Atom->Move (x, y, z);
}

void Fragment::OnChangeAtom ()
{
	if (m_bLoading || !m_Atom)
		return;
	Document *pDoc = (Document*) GetDocument ();
	if (!pDoc) return;
	string sym = m_Atom->GetSymbol ();
	m_TextItem->ReplaceText (sym, m_BeginAtom, m_EndAtom - m_BeginAtom);
	m_EndAtom = m_BeginAtom + sym.length ();
	// FIXME: we probably need to insert a tag there
	// Set the selection at the cursor position to avoid further atoms changes
	m_StartSel = m_EndSel = m_TextItem->GetCursorPosition ();
	OnChanged (false);
}

int Fragment::GetElementAtPos (unsigned start, unsigned &end)
{
	int Z;
	char text[4];
	memset (text, 0, 4);
	strncpy (text, m_buf.c_str () + start, 3);
	for (unsigned i = MIN (strlen (text), end - start); i > 0; i--) {
		text[i] = 0;
		if ((Z = Element::Z (text))) {
			end = start + i;
			return Z;
		}
	}
	return 0;
}

gccv::Anchor Fragment::GetChargePosition (FragmentAtom *pAtom, unsigned char &Pos, G_GNUC_UNUSED double Angle, double &x, double &y)
{
	if ((pAtom != m_Atom) || (m_Atom->GetZ() == 0))
		return gccv::AnchorCenter;
	double width, height;
	Document* pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	if (!pDoc)
		return gccv::AnchorCenter;
	if (!m_TextItem)
		return gccv::AnchorCenter;
	/* search for charges */
	list <gccv::TextTag *> const *tags = m_TextItem->GetTags ();
	list <gccv::TextTag *>::const_iterator i, iend = tags->end ();
	for (i = tags->begin (); i != iend; i++)
		if ((*i)->GetTag () == ChargeTag)
			return gccv::AnchorCenter; //localized charges are prohibited if a global charge already exists
	// Get atom bounds
	int result = 0xff;
	gccv::Rect rect;
	m_TextItem->GetPositionAtIndex (m_BeginAtom, rect);
	x = rect.x0;
	if (m_BeginAtom != 0)
		result &= 0x6d;
	m_TextItem->GetPositionAtIndex (m_EndAtom, rect);
	width = rect.x0 - x;
	if (m_EndAtom < m_buf.length ())
		result &= 0xb6;
	width /= pTheme->GetZoomFactor ();
	height = m_height / pTheme->GetZoomFactor (); // hmm, we might find something better
	if (m_Atom->GetBondsNumber()) {
		map < gcu::Bondable *, gcu::Bond * >::iterator j;
		Bond* pBond = (Bond*)m_Atom->GetFirstBond (j);
		double angle = pBond->GetAngle2D (m_Atom) + 180.0;
		if ((result & POSITION_NE) && (angle >= 180.0) && (angle <= 270.0))
			result -= POSITION_NE;
		if ((result & POSITION_NW) && (((angle >= 270.0) && (angle <= 360.0)) || (fabs(angle) < 0.1)))
			result -= POSITION_NW;
		if ((result & POSITION_N) && (angle >= 225.0) && (angle <= 315.0))
			result -= POSITION_N;
		if ((result & POSITION_SE) && (angle >= 90.0) && (angle <= 180.0))
			result -= POSITION_SE;
		if ((result & POSITION_SW) && (((angle >= 0.0) && (angle <= 90.0)) || (fabs(angle - 360.0) < 0.1)))
			result -= POSITION_SW;
		if ((result & POSITION_S) && (angle >= 45.0) && (angle <= 135.0))
			result -= POSITION_S;
		if ((result & POSITION_E) && ((angle <= 225.0) && (angle >= 135.0)))
			result -= POSITION_E;
		if ((result & POSITION_W) && ((angle >= 315.0) || (angle <= 45.0)))
			result -= POSITION_W;
	}
	if (Pos == 0xff) {
		if (result) {
			if (result & POSITION_NE)
				Pos = POSITION_NE;
			else if (result & POSITION_NW)
				Pos = POSITION_NW;
			else if (result & POSITION_N)
				Pos = POSITION_N;
			else if (result & POSITION_SE)
				Pos = POSITION_SE;
			else if (result & POSITION_SW)
				Pos = POSITION_SW;
			else if (result & POSITION_S)
				Pos = POSITION_S;
			else if (result & POSITION_E)
				Pos = POSITION_E;
			else if (result & POSITION_W)
				Pos = POSITION_W;
		} else
			return gccv::AnchorCenter;
	} else if (Pos) {
		if (!(Pos & result))
			return gccv::AnchorCenter;
	} else
		return gccv::AnchorCenter;

	switch (Pos) {
	case POSITION_NE:
		x = m_x + width / 2.0;
		y = m_y - height / 2.0;
		return gccv::AnchorWest;
	case  POSITION_NW:
		x = m_x - width / 2.0;
		y = m_y - height / 2.0;
		return gccv::AnchorEast;
	case  POSITION_N:
		x = m_x;
		y = m_y - height / 2.0;
		return gccv::AnchorSouth;
	case POSITION_SE:
		x = m_x + width / 2.0;
		y = m_y + height / 2.0;
		return gccv::AnchorWest;
	case POSITION_SW:
		x = m_x - width / 2.0;
		y = m_y + height / 2.0;
		return gccv::AnchorEast;
	case POSITION_S:
		x = m_x;
		y = m_y + height / 2.0;
		return gccv::AnchorNorth;
	case POSITION_E:
		x = m_x + width / 2.0;
		y = m_y;
		return gccv::AnchorWest;
	case POSITION_W:
		x = m_x - width / 2.0;
		y = m_y;
		return gccv::AnchorEast;
	}
	return gccv::AnchorCenter;
}

bool Fragment::Validate ()
{
	char const *charge;
	char *err;
	if ((m_buf.length () == 0)
		&& m_Atom->GetBondsNumber () == 0)
		return true;
	if (m_Atom->GetZ() == 0 || (dynamic_cast <FragmentResidue*> (m_Atom) && !((FragmentResidue*) m_Atom)->GetResidue ())) {
		Document *pDoc = dynamic_cast <Document*> (GetDocument ());
		m_TextItem->SetSelectionBounds (m_BeginAtom, (m_EndAtom == m_BeginAtom)? m_EndAtom + 1: m_EndAtom);
		GtkWidget* w = gtk_message_dialog_new (
										GTK_WINDOW (static_cast < Window *> (pDoc->GetWindow ())->GetWindow ()),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
										_("Invalid symbol."));
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);
		return false;
	}
	//now scan for charges and stoichiometric numbers and validate
//	gccv::TextTagList chargetags; // FIXME: don't use for now
	list <gccv::TextTag *> const *tags = m_TextItem->GetTags ();
	list <gccv::TextTag *>::const_iterator it, itend = tags->end ();
	for (it = tags->begin (); it != itend; it++) {
		if ((*it)->GetTag () == ChargeTag) {
			charge = m_buf.c_str () + (*it)->GetStartIndex ();
			strtol (charge, &err, 10);
			if (err == charge) {
				// we allow for any alphabetic unicode character
				if (g_unichar_isalpha (g_utf8_get_char (err)))
					err = g_utf8_next_char (err);
			}
			unsigned length = (*it)->GetEndIndex () - (*it)->GetStartIndex () - (err - charge);
			if ((*err == '+' && length == 1) ||
			    (!strncmp (err, "−", strlen ("−")) && length == strlen ("−")))
				continue;
			// if we are there, we have an error
			Document *pDoc = dynamic_cast<Document*> (GetDocument ());
			m_TextItem->SetSelectionBounds ((*it)->GetStartIndex (), (*it)->GetEndIndex ());
			GtkWidget* w = gtk_message_dialog_new (
											GTK_WINDOW (static_cast <Window * > (pDoc->GetWindow ())->GetWindow ()),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
											_("Invalid charge."));
			gtk_dialog_run(GTK_DIALOG(w));
			gtk_widget_destroy(w);
			return false;
		}
	}
	// we get everything displayed as superscript and see if it is a charge
	// all text not recognized as a charge will be analyzed through the gcu::Formula mechanism
	// FIXME: write that code
	return true;
}

void Fragment::Transform2D (Matrix2D& m, double x, double y)
{
	m_x -= x;
	m_y -= y;
	m.Transform (m_x, m_y);
	m_x += x;
	m_y += y;
	m_Atom->SetCoords (m_x, m_y);
}

double Fragment::GetYAlign () const
{
	return m_y;
}

int Fragment::GetAvailablePosition (G_GNUC_UNUSED double& x, G_GNUC_UNUSED double& y)
{
	return 0;
}

bool Fragment::GetPosition (G_GNUC_UNUSED double angle, G_GNUC_UNUSED double& x, G_GNUC_UNUSED double& y)
{
	return false;
}

bool Fragment::SetProperty (unsigned property, char const *value)
{
	m_bLoading = true;
	switch (property) {
	case GCU_PROP_POS2D: {
		std::istringstream str (value);
		str >> m_x >> m_y;
		gcu::Document *doc = GetDocument ();
		if (doc) {
			m_x *= doc->GetScale ();
			m_y *= doc->GetScale ();
		}
		m_Atom->SetCoords (m_x, m_y);
		break;
	}
	case GCU_PROP_X: {
		std::istringstream str (value);
		str >> m_x;
		gcu::Document *doc = GetDocument ();
		if (doc)
			m_x *= doc->GetScale ();
		m_Atom->SetCoords (m_x, m_y);
		break;
	}
	case GCU_PROP_Y: {
		std::istringstream str (value);
		str >> m_y;
		gcu::Document *doc = GetDocument ();
		if (doc)
			m_y *= doc->GetScale ();
		m_Atom->SetCoords (m_x, m_y);
		break;
	}
	case GCU_PROP_TEXT_TEXT: {
		m_buf = value;
		if (m_EndAtom > m_BeginAtom) {
			Residue *r = NULL;
			char sy[Residue::MaxSymbolLength + 1];
			strncpy (sy, m_buf.c_str () + m_BeginAtom, Residue::MaxSymbolLength);
			int i = Residue::MaxSymbolLength;
			while (i > 0) {
				sy[i] = 0;
				r = (Residue *) Residue::GetResidue (sy, NULL);
				if (r)
					break;
				i--;
			}
			if (r) {
				m_EndAtom = m_BeginAtom +  + strlen (sy);
				map < gcu::Bondable *, gcu::Bond * >::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				pOldAtom->SetParent (NULL);
				m_Atom = new FragmentResidue (this, sy);
				AddChild (m_Atom);
				m_Atom->SetId ((char *) pOldAtom->GetId ());
				m_Atom->SetCoords (m_x, m_y);
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				delete pOldAtom;
			} else {
				int Z = GetElementAtPos (m_BeginAtom, m_EndAtom);
				if (Z)
					m_Atom->SetZ (Z);
			}
			Analyze ();
		}
		break;
	}
	case GCU_PROP_FRAGMENT_ATOM_START:
		m_BeginAtom = atoi (value);
		m_EndAtom = m_BeginAtom + Residue::MaxSymbolLength;
		if (m_buf.length ()) {
			Residue *r = NULL;
			char sy[Residue::MaxSymbolLength + 1];
			strncpy (sy, m_buf.c_str () + m_BeginAtom, Residue::MaxSymbolLength);
			int i = Residue::MaxSymbolLength;
			while (i > 0) {
				sy[i] = 0;
				r = (Residue *) Residue::GetResidue (sy, NULL);
				if (r)
					break;
				i--;
			}
			if (r) {
				m_EndAtom = m_BeginAtom + strlen (sy);
				map < gcu::Bondable *, gcu::Bond * >::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				pOldAtom->SetParent (NULL);
				m_Atom = new FragmentResidue (this, sy);
				AddChild (m_Atom);
				m_Atom->SetId ((char *) pOldAtom->GetId ());
				m_Atom->SetCoords (m_x, m_y);
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				delete pOldAtom;
			} else {
				int Z = GetElementAtPos (m_BeginAtom, m_EndAtom);
				if (Z)
					m_Atom->SetZ (Z);
			}
			Analyze ();
		}
		break;
	case GCU_PROP_FRAGMENT_ATOM_ID:
		m_Atom->SetId (value);
		if (!m_Atom->GetParent ())
			AddChild (m_Atom);
		break;
	default:
		break;
	}
	m_bLoading = false;
	return true;
}

std::string Fragment::GetProperty (unsigned property) const
{
	switch (property) {
	case GCU_PROP_POS2D: {
		std::ostringstream str;
		gcu::Document *doc = GetDocument ();
		double scale = (doc)? doc->GetScale (): 1.;
		str << m_x / scale << m_y / scale;
		return str.str ();
	}
	case GCU_PROP_X: {
		std::ostringstream str;
		gcu::Document *doc = GetDocument ();
		double scale = (doc)? doc->GetScale (): 1.;
		str << m_x / scale;
		return str.str ();
	}
	case GCU_PROP_Y: {
		std::ostringstream str;
		gcu::Document *doc = GetDocument ();
		double scale = (doc)? doc->GetScale (): 1.;
		str << m_y / scale;
		return str.str ();
	}
	case GCU_PROP_FRAGMENT_ATOM_ID:
			return m_Atom->GetId ();
		break;
	default:
		break;
	}
	return TextObject::GetProperty (property);
}

bool Fragment::Analyze () {
	// search if main atom is at start or at end
	m_Inversable = (m_BeginAtom == 0 || m_EndAtom == m_buf.size ()) && ((m_EndAtom - m_BeginAtom) < m_buf.length ());
//	int valence = m_Atom->GetValence ();
	AnalContent ();
	return true;
}

bool Fragment::Update () {
	if (m_Atom->GetBondsNumber () > 0 && m_Inversable) {
		map < gcu::Bondable *, gcu::Bond *>::iterator i;
		Bond *bond = reinterpret_cast <Bond *> (m_Atom->GetFirstBond (i));
		double angle = bond->GetAngle2D (m_Atom);
		if (m_BeginAtom == 0 && (angle < 89. && angle > -89.)) {
			// build the formula, then write elements in reverse order might be unsecure in some cases (if linked atom has a stoichiometric coefficient)
			Formula *formula = new Formula (m_buf, GCU_FORMULA_PARSE_RESIDUE);
			std::list<FormulaElt *> const &elts = formula->GetElements ();
			m_buf.clear ();
			std::list<FormulaElt *>::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++) {
				m_buf += (*i)->Text ();
			}
			delete formula;
			m_EndAtom = m_buf.length ();
			m_BeginAtom = m_EndAtom - strlen (m_Atom->GetSymbol ());
			AnalContent ();
			return true;
		} else if (m_BeginAtom > 0 && (angle > 91. || angle < -91.)) {
			// build the formula, then write elements in reverse order might be unsecure in some cases (if linked atom has a stoichiometric coefficient)
			Formula *formula = new Formula (m_buf, GCU_FORMULA_PARSE_RESIDUE);
			std::list<FormulaElt *> const &elts = formula->GetElements ();
			m_buf.clear ();
			std::list<FormulaElt *>::const_reverse_iterator i, end = elts.rend ();
			for (i = elts.rbegin (); i!= end; i++) {
				m_buf += (*i)->Text ();
			}
			delete formula;
			m_BeginAtom = 0;
			m_EndAtom = strlen (m_Atom->GetSymbol ());
			AnalContent ();
			return true;
		}
	}
	return false;
}

gccv::Item *Fragment::GetChargeItem ()
{
	return (m_Atom)? m_Atom->GetChargeItem (): NULL;
}

std::string Fragment::Name ()
{
	return _("Fragment");
}

bool Fragment::GetCoords (double *x, double *y, double *z) const
{
	if (x == NULL || y == NULL)
		return false;
	*x = m_x;
	*y = m_y;
	if (z)
		*z = 0.;
	return true;
}

}	//	namespace gcp
