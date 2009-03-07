// -*- C++ -*-

/* 
 * GChemPaint library
 * fragment.cc 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gccv/structs.h>
#include <gccv/text.h>
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/objprops.h>
#include <pango/pango-attributes.h>
#include <glib/gi18n-lib.h>
#include <list>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

static void on_fragment_changed (Fragment *fragment)
{
	fragment->OnChanged (true);
}

static void on_fragment_sel_changed (Fragment *fragment, gccv::TextSelBounds *bounds)
{
	fragment->OnSelChanged (bounds);
}

Fragment::Fragment ():
	TextObject (FragmentType),
	m_Inversable (false)
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
	m_Inversable (false)
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
/*	View* pView = pDoc->GetView ();
	GtkWidget* pWidget = pView->GetWidget ();
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
	GnomeCanvasGroup *group = pData->Items[this];
	if (!group) {
		pData->Items.erase (this);
		m_bLoading = false;
		return false;
	}
	GnomeCanvasPango *PangoItem = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (group), "fragment"));
	unsigned CurPos = gnome_canvas_pango_get_cur_index (PangoItem);
	AnalContent (m_StartSel, CurPos);
	m_bLoading = true;
	m_buf = pango_layout_get_text (m_Layout);
	if (m_buf.length ()) {
		PangoLayoutIter *iter = pango_layout_get_iter (m_Layout);
		m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
	}*/
	/*main atom management*/
/*	FragmentResidue *residue = dynamic_cast <FragmentResidue*> (m_Atom);
	Residue *r = NULL;
	char sy[Residue::MaxSymbolLength + 1];
	if (!m_Atom->GetSymbol ())
		m_StartSel = m_BeginAtom;
	if (!m_Atom->GetZ () || (residue != NULL && residue->GetResidue () == NULL)) {
		strncpy (sy, m_buf.c_str () + m_StartSel, Residue::MaxSymbolLength);
		int i = Residue::MaxSymbolLength;
		while (i > 0) {
			sy[i] = 0;
			r = (Residue *) Residue::GetResidue (sy, NULL);
			if (r || Element::Z (sy) > 0)
				break;
			i--;
		}
		if (r) {
			CurPos = m_StartSel + i;
			m_BeginAtom = m_StartSel;
			m_EndAtom = CurPos;
			if (residue)
				residue->SetResidue (r);
			else {
				map<gcu::Atom*, gcu::Bond*>::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentResidue (this, sy);
				m_Atom->SetId ((gchar*) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				delete pOldAtom;
			}
		} else {
			int Z = GetElementAtPos (m_StartSel, CurPos);
			if (!Z && m_StartSel > m_BeginAtom)
				Z = GetElementAtPos (m_StartSel = m_BeginAtom, CurPos);
			if (Z) {
				if (residue) {
					map<gcu::Atom*, gcu::Bond*>::iterator i;
					Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
					Atom *pOldAtom = m_Atom;
					m_Atom = new FragmentAtom (this, Z);
					m_Atom->SetId ((gchar*) pOldAtom->GetId ());
					if (pBond) {
						pBond->ReplaceAtom (pOldAtom, m_Atom);
						m_Atom->AddBond (pBond);
					}
					delete pOldAtom;
				} else
					m_Atom->SetZ (Z);
				m_BeginAtom = m_StartSel;
				m_EndAtom = CurPos;
			}
		}
	} else if (m_EndSel <= m_BeginAtom) {
		int delta = CurPos - m_EndSel;
		m_BeginAtom += delta;
		m_EndAtom += delta;
	} else if ((m_EndAtom <= m_EndSel && m_EndAtom >= m_StartSel) ||
		(m_BeginAtom <= m_EndSel && m_BeginAtom >= m_StartSel) ||
		(m_BeginAtom + Residue::MaxSymbolLength >= CurPos)) {
		if (m_BeginAtom > m_StartSel)
			m_BeginAtom = m_StartSel;
		if (m_EndAtom > CurPos)
			m_EndAtom = CurPos;
		else if (m_EndAtom < m_BeginAtom + Residue::MaxSymbolLength)
			m_EndAtom = m_BeginAtom + Residue::MaxSymbolLength;
		strncpy (sy, m_buf.c_str () + m_BeginAtom, Residue::MaxSymbolLength);
		int i = Residue::MaxSymbolLength;
		while (i > 0) {
			sy[i] = 0;
			r = (Residue *) Residue::GetResidue (sy, NULL);
			if (r || Element::Z (sy))
				break;
			i--;
		}
		if (r) {
			m_EndAtom = m_BeginAtom + i;
			if (residue)
				residue->SetResidue (r);
			else {
				map<gcu::Atom*, gcu::Bond*>::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentResidue (this, sy);
				m_Atom->SetId ((gchar*) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				delete pOldAtom;
			}
		} else {
			int Z = GetElementAtPos (m_BeginAtom, m_EndAtom);
			if (residue) {
				map<gcu::Atom*, gcu::Bond*>::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				m_Atom = new FragmentAtom (this, Z);
				m_Atom->SetId ((gchar*) pOldAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pOldAtom, m_Atom);
					m_Atom->AddBond (pBond);
				}
				delete pOldAtom;
			} else
				m_Atom->SetZ (Z);
			if (!Z)
				m_EndAtom = CurPos;
		}
	}
	PangoRectangle rect;
	pango_layout_index_to_pos (m_Layout, m_BeginAtom, &rect);
	m_lbearing = rect.x / PANGO_SCALE;
	pango_layout_index_to_pos (m_Layout, m_EndAtom, &rect);
	m_lbearing += rect.x / PANGO_SCALE;
	m_lbearing /=  2;
	pView->Update (this);
	m_bLoading = false;
	Window* pWin = pDoc->GetWindow ();
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
	pango_layout_get_extents (m_Layout, NULL, &rect);
	m_length = rect.width / PANGO_SCALE;
	m_height = rect.height / PANGO_SCALE;
	pView->Update (this);
	EmitSignal (OnChangedSignal);
	m_StartSel = m_EndSel = CurPos;
	if (m_buf.length () == 0) {
		m_BeginAtom = m_EndAtom = 0;
	}
	if (save) {
		Tool* FragmentTool = dynamic_cast<Application*> (pDoc->GetApplication ())->GetTool ("Fragment");
		if (!FragmentTool)
			return  true;
		xmlNodePtr node = SaveSelected ();
		if (node)
			FragmentTool->PushNode (node);
	}*/
	return true;
}

void Fragment::AddItem ()
{
}

void Fragment::UpdateItem ()
{
}

/*void Fragment::Add (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] != NULL)
		return;
	View* pView = pData->m_View;
	Theme *pTheme = pView->GetDoc ()->GetTheme ();
	if (m_ascent <= 0) {
		PangoContext* pc = gccv::Text::GetContext ();
		const_cast <Fragment *> (this)->m_Layout = pango_layout_new (pc);
		PangoAttrList *l = pango_attr_list_new ();
		pango_layout_set_attributes (m_Layout, l);
		pango_layout_set_font_description (m_Layout, pView->GetPangoFontDesc ());
		pango_layout_set_text (m_Layout, "l", 1);
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		const_cast <Fragment *> (this)->m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		pango_layout_set_text (m_Layout, "C", 1);
		PangoRectangle rect;
		pango_layout_get_extents (m_Layout, &rect, NULL);
		const_cast <Fragment *> (this)->m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
		pango_layout_set_text (m_Layout, m_buf.c_str (), -1);
		if (m_AttrList) {
			pango_layout_set_attributes (m_Layout, m_AttrList);
			pango_attr_list_unref (m_AttrList);
			const_cast <Fragment *> (this)->m_AttrList = NULL;
		}
		if (m_buf.length () > 0) {
			const_cast <Fragment *> (this)->m_buf.clear ();
			pango_layout_index_to_pos (m_Layout, m_BeginAtom, &rect);
			const_cast <Fragment *> (this)->m_lbearing = rect.x / PANGO_SCALE;
			pango_layout_index_to_pos (m_Layout, m_EndAtom, &rect);
			const_cast <Fragment *> (this)->m_lbearing += rect.x / PANGO_SCALE;
			const_cast <Fragment *> (this)->m_lbearing /=  2;
			iter = pango_layout_get_iter (m_Layout);
			const_cast <Fragment *> (this)->m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
			pango_layout_iter_free (iter);
		}
		pango_layout_get_extents (m_Layout, NULL, &rect);
		const_cast <Fragment *> (this)->m_length = rect.width / PANGO_SCALE;
		const_cast <Fragment *> (this)->m_height = rect.height / PANGO_SCALE;
	}
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL)), *chgp;
	GnomeCanvasItem* item = gnome_canvas_item_new(
						group,
						gnome_canvas_rect_ext_get_type (),
						"x1", m_x * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_lbearing,
						"y1", m_y * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_ascent + m_CHeight,
						"x2", m_x * pTheme->GetZoomFactor () + m_length + pTheme->GetPadding () - m_lbearing,
						"y2", m_y * pTheme->GetZoomFactor () + m_height + pTheme->GetPadding () - m_ascent + m_CHeight,
						NULL);
	gnome_canvas_item_hide (item);
	g_object_set_data (G_OBJECT (group), "rect", item);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
	item = gnome_canvas_item_new (
						group,
						gnome_canvas_pango_get_type (),
						"layout", m_Layout,
						"x", m_x * pTheme->GetZoomFactor () - m_lbearing,
						"y", m_y * pTheme->GetZoomFactor () - m_ascent + m_CHeight,
						"editing", false,
						NULL);
	g_object_set_data (G_OBJECT (group), "fragment", item);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_signal_connect_swapped (G_OBJECT (item), "changed", G_CALLBACK (on_fragment_changed), (void *) this);
	g_signal_connect_swapped (G_OBJECT (item), "sel-changed", G_CALLBACK (on_fragment_sel_changed), (void *) this);*/
	/* add charge */
	/*int charge = m_Atom->GetCharge ();
	if (charge) {
		double x, y, Angle, Dist;
		unsigned char Pos = m_Atom->Atom::GetChargePosition (&Angle, &Dist);
		int align = const_cast <Fragment *> (this)->GetChargePosition (m_Atom, Pos, 0., x, y);
		if (Dist != 0.) {
			x = m_x + Dist * cos (Angle);
			y = m_y - Dist * sin (Angle);
		}
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
		switch (align) {
		case -2:
			x -= pTheme->GetChargeSignSize () / 2.;
			y += pTheme->GetChargeSignSize () / 2.;
			break;
			case -1:
				x -= pTheme->GetChargeSignSize () + pTheme->GetPadding ();
				break;
			case 0:
			case -3:
				x -= pTheme->GetChargeSignSize () / 2.;
				break;
			case 1:
				x += pTheme->GetPadding ();
				break;
		case 2:
			x -= pTheme->GetChargeSignSize () / 2.;
			y -= pTheme->GetChargeSignSize () / 2.;
			break;
		}
		y -= pTheme->GetChargeSignSize () / 2.;
		chgp = (GnomeCanvasGroup*) gnome_canvas_item_new (
					group,
					gnome_canvas_group_ext_get_type (),
					NULL);
		g_object_set_data (G_OBJECT (group), "charge", chgp);
		item = gnome_canvas_item_new (
					chgp,
					gnome_canvas_ellipse_ext_get_type (),
					"x1", x,
					"y1", y,
					"x2", x + pTheme->GetChargeSignSize (),
					"y2", y + pTheme->GetChargeSignSize (),
					"outline_color", (pData->IsSelected (this))? SelectColor: Color,
					"width_units", 0.5,
					NULL
				);
		g_object_set_data (G_OBJECT (group), "circle", item);
		ArtBpath *path = art_new (ArtBpath, 5);
		path[0].code = ART_MOVETO_OPEN;
		path[0].x3 = x + 1.;
		path[1].code = ART_LINETO;
		path[1].x3 = x + pTheme->GetChargeSignSize () - 1.;
		path[0].y3 = path[1].y3 = y + pTheme->GetChargeSignSize () / 2.;
		if (charge > 0) {
			path[2].code = ART_MOVETO_OPEN;
			path[2].y3 = y + 1.;
			path[3].code = ART_LINETO;
			path[3].y3 = y + pTheme->GetChargeSignSize () - 1.;
			path[2].x3 = path[3].x3 = x + pTheme->GetChargeSignSize () / 2.;
			path[4].code = ART_END;
		} else
			path[2].code = ART_END;
		GnomeCanvasPathDef *cpd = gnome_canvas_path_def_new_from_bpath (path);
		item = gnome_canvas_item_new (
					chgp,
					gnome_canvas_bpath_ext_get_type (),
					"bpath", cpd,
					"outline_color", (pData->IsSelected(this))? SelectColor: Color,
					"width_units", 1.,
					NULL
				);
		gnome_canvas_path_def_unref (cpd);
		g_object_set_data (G_OBJECT (group), "sign", item);
	}
	pData->Items[this] = group;
	const_cast <FragmentAtom *> (m_Atom)->DoBuildSymbolGeometry (pData->m_View);
}*/

void Fragment::SetSelected (int state)
{
	GOColor chargecolor, color;
	bool visible = true;
	switch (state) {	
	case SelStateUnselected:
		color = NULL;
		chargecolor = RGBA_BLACK;
		visible = false;
		break;
	case SelStateSelected:
		chargecolor = color = SelectColor;
		break;
	case SelStateUpdating:
		chargecolor = color = AddColor;
		break;
	case SelStateErasing:
		chargecolor = color = DeleteColor;
		break;
	default:
		color = NULL;
		chargecolor = RGBA_BLACK;
		break;
	}
/*	gpointer item;
	item = g_object_get_data (G_OBJECT (group), "rect");
	g_object_set (G_OBJECT (item),
				"fill_color", color, NULL);
	if (visible)
		gnome_canvas_item_show (GNOME_CANVAS_ITEM (item));
	else
		gnome_canvas_item_hide (GNOME_CANVAS_ITEM (item));
	if ((item = g_object_get_data (G_OBJECT (group), "circle")))
		g_object_set (item, "outline_color", chargecolor, NULL);
	if ((item = g_object_get_data (G_OBJECT (group), "sign")))
		g_object_set (item, "outline_color", chargecolor, NULL);*/
}

/*void Fragment::Update (GtkWidget* w) const
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup *group = pData->Items[this];
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "fragment")),
						"x", m_x * pTheme->GetZoomFactor () - m_lbearing,
						"y", m_y * pTheme->GetZoomFactor () - m_ascent + m_CHeight,
						"width", m_length,
						"height", m_height,
						NULL);
	g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "rect")),
						"x1", m_x * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_lbearing,
						"y1", m_y * pTheme->GetZoomFactor () - pTheme->GetPadding () - m_ascent + m_CHeight,
						"x2", m_x * pTheme->GetZoomFactor () + m_length + pTheme->GetPadding () - m_lbearing,
						"y2", m_y * pTheme->GetZoomFactor () + m_height + pTheme->GetPadding () - m_ascent + m_CHeight,
						NULL);
	void* item = g_object_get_data (G_OBJECT (group), "charge");
	int charge = m_Atom->GetCharge ();
	if (charge) {
		double x, y, Angle, Dist;
		unsigned char Pos = m_Atom->Atom::GetChargePosition (&Angle, &Dist);
		if (item) {
			int align = const_cast <Fragment *> (this)->GetChargePosition (m_Atom, Pos, Angle, x, y);
			if (Dist != 0.) {
				x = m_x + Dist * cos (Angle);
				y = m_y - Dist * sin (Angle);
			}
			x *= pTheme->GetZoomFactor ();
			y *= pTheme->GetZoomFactor ();
			switch (align) {
			case -2:
				x -= pTheme->GetChargeSignSize () / 2.;
				y += pTheme->GetChargeSignSize () / 2.;
				break;
			case -1:
				x -= pTheme->GetChargeSignSize () + pTheme->GetPadding ();
				break;
			case 0:
			case -3:
				x -= pTheme->GetChargeSignSize () / 2.;
				break;
			case 1:
				x += pTheme->GetPadding ();
				break;
			case 2:
				x -= pTheme->GetChargeSignSize () / 2.;
				y -= pTheme->GetChargeSignSize () / 2.;
				break;
			}
			y -= pTheme->GetChargeSignSize () / 2.;
			item = g_object_get_data (G_OBJECT (group), "circle");
			g_object_set (G_OBJECT (item),
						"x1", x,
						"y1", y,
						"x2", x + pTheme->GetChargeSignSize (),
						"y2", y + pTheme->GetChargeSignSize (),
						NULL);
			item = g_object_get_data (G_OBJECT (group), "sign");
			ArtBpath *path = art_new (ArtBpath, 5);
			path[0].code = ART_MOVETO_OPEN;
			path[0].x3 = x + 1.;
			path[1].code = ART_LINETO;
			path[1].x3 = x + pTheme->GetChargeSignSize () - 1.;
			path[0].y3 = path[1].y3 = y + pTheme->GetChargeSignSize () / 2.;
			if (charge > 0) {
				path[2].code = ART_MOVETO_OPEN;
				path[2].y3 = y + 1.;
				path[3].code = ART_LINETO;
				path[3].y3 = y + pTheme->GetChargeSignSize () - 1.;
				path[2].x3 = path[3].x3 = x + pTheme->GetChargeSignSize () / 2.;
				path[4].code = ART_END;
			} else
				path[2].code = ART_END;
			GnomeCanvasPathDef *cpd = gnome_canvas_path_def_new_from_bpath (path);
			g_object_set (G_OBJECT (item),
						"bpath", cpd,
						NULL
					);
			gnome_canvas_path_def_unref (cpd);
		} else {
			GnomeCanvasGroup *chgp;
			int align = const_cast <Fragment *> (this)->GetChargePosition (m_Atom, Pos, Angle, x, y);
			x *= pTheme->GetZoomFactor ();
			if (Dist != 0.) {
				x = m_x + Dist * cos (Angle);
				y = m_y - Dist * sin (Angle);
			}
			y *= pTheme->GetZoomFactor ();
			switch (align) {
			case -2:
				x -= pTheme->GetChargeSignSize () / 2.;
				y += pTheme->GetChargeSignSize () / 2.;
				break;
			case -1:
				x -= pTheme->GetChargeSignSize () + pTheme->GetPadding ();
				break;
			case 0:
			case -3:
				x -= pTheme->GetChargeSignSize () / 2.;
				break;
			case 1:
				x += pTheme->GetPadding ();
				break;
			case 2:
				x -= pTheme->GetChargeSignSize () / 2.;
				y -= pTheme->GetChargeSignSize () / 2.;
				break;
			}
			y -= pTheme->GetChargeSignSize () / 2.;
			chgp = (GnomeCanvasGroup*) gnome_canvas_item_new (
						group,
						gnome_canvas_group_ext_get_type (),
						NULL);
			g_object_set_data (G_OBJECT (group), "charge", chgp);
			item = gnome_canvas_item_new (
						chgp,
						gnome_canvas_ellipse_ext_get_type (),
						"x1", x,
						"y1", y,
						"x2", x + pTheme->GetChargeSignSize (),
						"y2", y + pTheme->GetChargeSignSize (),
						"outline_color", (pData->IsSelected (this))? SelectColor: Color,
						"width_units", 0.5,
						NULL
					);
			g_object_set_data (G_OBJECT (group), "circle", item);
			ArtBpath *path = art_new (ArtBpath, 5);
			path[0].code = ART_MOVETO_OPEN;
			path[0].x3 = x + 1.;
			path[1].code = ART_LINETO;
			path[1].x3 = x + pTheme->GetChargeSignSize () - 1.;
			path[0].y3 = path[1].y3 = y + pTheme->GetChargeSignSize () / 2.;
			if (charge > 0) {
				path[2].code = ART_MOVETO_OPEN;
				path[2].y3 = y + 1.;
				path[3].code = ART_LINETO;
				path[3].y3 = y + pTheme->GetChargeSignSize () - 1.;
				path[2].x3 = path[3].x3 = x + pTheme->GetChargeSignSize () / 2.;
				path[4].code = ART_END;
			} else
				path[2].code = ART_END;
			GnomeCanvasPathDef *cpd = gnome_canvas_path_def_new_from_bpath (path);
			item = gnome_canvas_item_new (
						chgp,
						gnome_canvas_bpath_ext_get_type (),
						"bpath", cpd,
						"outline_color", (pData->IsSelected (this))? SelectColor: Color,
						"width_units", 1.,
						NULL
					);
			gnome_canvas_path_def_unref (cpd);
			g_object_set_data (G_OBJECT (group), "sign", item);
		}
	} else {
		if (item) {
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data ((GObject*) group, "charge", NULL);
		}
	}
	const_cast <FragmentAtom *> (m_Atom)->DoBuildSymbolGeometry (pData->m_View);
}*/

xmlNodePtr Fragment::Save (xmlDocPtr xml) const
{
	const_cast <Fragment *> (this)->m_buf = pango_layout_get_text (m_Layout);
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

struct FilterStruct {
	unsigned start, end;
	list<PangoAttribute*> pal;
};

bool filter_func (PangoAttribute *attribute, struct FilterStruct *s)
{
	if (attribute->klass->type == PANGO_ATTR_RISE && ((PangoAttrInt*) attribute)->value > 0 &&
		s->start <= attribute->start_index && s->end >= attribute->end_index) {
		list<PangoAttribute*>::iterator i, iend = s->pal.end ();
		for (i = s->pal.begin (); i != iend; i++)
			if ((*i)->start_index > attribute->end_index)
				break;
		s->pal.insert (i, attribute);
	}
	return FALSE;
}

bool Fragment::SavePortion (xmlDocPtr xml, xmlNodePtr node, unsigned start, unsigned end) const
{
	xmlNodePtr child;
	struct FilterStruct s;
	s.start = start;
	s.end = end;
	int charge;
	if (m_AttrList == NULL)
		const_cast <Fragment *> (this)->m_AttrList = pango_layout_get_attributes (m_Layout);
	pango_attr_list_filter (m_AttrList, (PangoAttrFilterFunc) filter_func, &s);
	list<PangoAttribute*>::iterator i, iend = s.pal.end ();
	string str;
	char *err;
	for (i = s.pal.begin (); i != iend; i++) {
		if (start < (*i)->start_index) {
			str.assign (m_buf, start, (*i)->start_index - start);
			xmlNodeAddContent (node, (const xmlChar*) str.c_str ());
		}
		str.assign (m_buf, (*i)->start_index, (*i)->end_index - (*i)->start_index);
		child = xmlNewDocNode (xml, NULL, (xmlChar*) "charge", NULL);
		if (!child)
			return false;
		charge = strtol (str.c_str (), &err, 10);
		if (err && strcmp (err, "+") && strcmp (err, "-")) {
			if (!m_RealSave) {
				return  false;
				xmlFreeNode (child);
			}
			Document *pDoc = (Document*) GetDocument ();
			GtkWidget* w = gtk_message_dialog_new (
											pDoc->GetWindow ()->GetWindow (),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
											_("Invalid charge."));
			gtk_window_set_icon_name (GTK_WINDOW (w), "gchempaint");
			gtk_dialog_run (GTK_DIALOG (w));
			gtk_widget_destroy (w);
			return false;
		} else {
			if (!charge)
				charge = 1;
			if (*err == '-')
				charge = - charge;
			char *buf = g_strdup_printf ("%d", charge);
			xmlNewProp (child, (xmlChar*) "value", (xmlChar*) buf);
			g_free (buf);
			xmlAddChild (node, child);
		}
		start = (*i)->end_index;
	}
	if (start < end) {
		str.assign (m_buf, start, end - start);
		xmlNodeAddContent (node, (const xmlChar*) str.c_str ());
	}
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
	if (m_AttrList != NULL)
		pango_attr_list_unref (m_AttrList);
	m_AttrList = pango_attr_list_new ();
	m_bLoading = true;
	m_buf.clear (); // just in case
	xmlNodePtr child = node->children;
	char* tmp;
	PangoAttribute *attr;
	int size = pTheme->GetFontSize ();
	while (child) {
		if (!strcmp ((const char*) child->name, "text")) {
			tmp = (char*) xmlNodeGetContent (child);
			m_buf += tmp;
			xmlFree (tmp);
		} else if (!strcmp ((const char*) child->name, "atom")) {
			if (!m_Atom->Load (child))
				return false;
			m_BeginAtom = m_buf.length ();
			m_buf += m_Atom->GetSymbol();
			m_Atom->SetCoords (m_x, m_y);
			m_EndAtom = m_buf.length ();
		} else if (!strcmp ((const char*) child->name, "residue")) {
			// replace the atom by a residue
			map<gcu::Atom*, gcu::Bond*>::iterator i;
			Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
			Atom *pOldAtom = m_Atom;
			m_Atom = new FragmentResidue (this, NULL);
			if (pBond) {
				pBond->ReplaceAtom (pOldAtom, m_Atom);
				m_Atom->AddBond (pBond);
			}
			delete pOldAtom;
			char id[8];
			int j = 1;
			id[0] = 'a';
			do
				snprintf (id+1, 7, "%d", j++);
			while (pDoc->GetDescendant (id) != NULL);
			m_Atom->SetId (id);
			AddChild (m_Atom);
			if (!m_Atom->Load (child))
				return false;
			m_BeginAtom = m_buf.length ();
			m_buf += m_Atom->GetSymbol ();
			m_Atom->SetCoords (m_x, m_y);
			m_EndAtom = m_buf.length ();
		} else if (!strcmp ((const char*) child->name, "charge")) {
			int start = m_buf.length (), end;
			tmp = (char*) xmlGetProp (child, (xmlChar*) "value");
			int charge = atoi (tmp);
			xmlFree (tmp);
			if (abs(charge) > 1)
				tmp = g_strdup_printf ("%d%c", abs (charge), (charge > 0)? '+': '-');
			else if (charge == 1)
				tmp = g_strdup ("+");
			else if (charge == -1)
				tmp = g_strdup ("-");
			else
				tmp = g_strdup ("");//should not occur!
			m_buf += tmp;
			end = m_buf.length ();
			attr = pango_attr_size_new (size * 2 / 3);
			attr->start_index = start;
			attr->end_index = end;
			pango_attr_list_insert (m_AttrList, attr);
			attr = pango_attr_rise_new (2 * size / 3);
			attr->start_index = start;
			attr->end_index = end;
			pango_attr_list_insert (m_AttrList, attr);
		}
		child = child->next;
	}
	if (m_Layout) {
		pango_layout_set_text (m_Layout, m_buf.c_str (), -1);
		pango_layout_set_attributes (m_Layout, m_AttrList);
	}
	AnalContent ();
	m_bLoading = false;
	return true;
}

void Fragment::AnalContent ()
{
	if (!m_Atom->GetParent ())
		AddChild (m_Atom);
	unsigned end = (m_Layout)? strlen (pango_layout_get_text (m_Layout)): m_buf.length ();
	AnalContent(0, end);
}

typedef struct
{
	unsigned index, end;
	bool result;
} ChargeFindStruct;

static bool search_for_charge (PangoAttribute *attr, ChargeFindStruct *s)
{
	if (attr->start_index <= s->index && attr->end_index >= s->index
		&& attr->klass->type == PANGO_ATTR_RISE &&
		((PangoAttrInt*) attr)->value > 0) {
			s->result = true;
			s->index = attr->start_index;
			s->end = attr->end_index;
		}
	return false;
}

void Fragment::AnalContent (unsigned start, unsigned &end)
{
	Document* pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return;
	Theme *pTheme = pDoc->GetTheme ();
	char const *text;
	PangoAttrList *l;
	if (m_Layout) {
		text = pango_layout_get_text (m_Layout);
		l = pango_layout_get_attributes (m_Layout);
	} else {
		text = m_buf.c_str ();
		l = pango_attr_list_ref (m_AttrList);
	}
	bool Charge = false;
	unsigned start_tag, end_tag, next;
	char c;
	ChargeFindStruct s;
	s.index = s.end = 0;
	if (start > 0) {
		// search if a charge is at preceeding character
		s.result = false;
		s.index = start;
		pango_attr_list_filter (l, (PangoAttrFilterFunc) search_for_charge, &s);
		Charge = s.result;
	} else if (text[start] == '+' || text[start] == '-')
		Charge = true;
	else
		Charge = false;
	next = start;
	start_tag = end_tag = start;
	while (start < end) {
		c = text[start];
		if ((c >= '0') && (c <= '9')) {
			s.result = false;
			s.index = start;
			pango_attr_list_filter (l, (PangoAttrFilterFunc) search_for_charge, &s);
			Charge = s.result;
			next = start + 1;
			// add new size and rise attributes
			int size = pTheme->GetFontSize ();
			PangoAttribute *attr = pango_attr_size_new (size * 2 / 3);
			attr->start_index = start;
			attr->end_index = next;
			pango_attr_list_change (l, attr);
			if (!Charge)
				attr = pango_attr_rise_new (-size / 3);
			else {
				if (text[start - 1] == '+' || text[start - 1] == '-') {
					// move character before sign
					char *new_t = g_strdup (text);
					new_t[start] = new_t[start - 1];
					new_t[start - 1] = c;
					if (m_Layout) {
						pango_layout_set_text (m_Layout, new_t, -1);
						text = pango_layout_get_text (m_Layout);
					} else
						m_buf = new_t;
						text = m_buf.c_str ();
				}
				attr = pango_attr_rise_new (size * 2 / 3);
			}
			attr->start_index = start;
			attr->end_index = next;
				pango_attr_list_change (l, attr);
			start = next - 1;
		} else if ((c == '+') || (c == '-')) {
			if (!m_bLoading) {
				//do not allow both local and global charges
				if (m_Atom->GetCharge ())
					m_Atom->SetCharge (0);
				next = start + 1;
				if (!Charge) {
					Charge = true;
					int size = pTheme->GetFontSize ();
					PangoAttribute *attr = pango_attr_size_new (size * 2 / 3);
					attr->start_index = start;
					attr->end_index = next;
					pango_attr_list_change (l, attr);
					attr = pango_attr_rise_new (2 * size / 3);
					attr->start_index = start;
					attr->end_index = next;
					pango_attr_list_change (l, attr);
				} else {
					string old_charge (m_buf, s.index, s.end - s.index);
					char *nextch = NULL;
					int charge = strtol (old_charge.c_str (), &nextch, 10);
					if (charge == 0)
						charge = 1;
					if (*nextch == 0) {
						// no sign, just add it
						int size = pTheme->GetFontSize ();
						PangoAttribute *attr = pango_attr_size_new (size * 2 / 3);
						attr->start_index = start;
						attr->end_index = next;
						pango_attr_list_change (l, attr);
						attr = pango_attr_rise_new (2 * size / 3);
						attr->start_index = start;
						attr->end_index = next;
						pango_attr_list_change (l, attr);
					} else {
						if (*nextch == '-')
							charge = - charge;
						if (c == '+')
							charge++;
						else
							charge--;
						char *buf;
						if (abs(charge) > 1)
							buf = g_strdup_printf ("%d%c", abs (charge), (charge > 0)? '+': '-');
						else if (charge == 1)
							buf = g_strdup ("+");
						else if (charge == -1)
							buf = g_strdup ("-");
						else
							buf = g_strdup ("");
						int size = pTheme->GetFontSize ();
						PangoAttrList *l = pango_attr_list_new ();
						PangoAttribute *attr = pango_attr_size_new (size * 2 / 3);
						pango_attr_list_insert (l, attr);
						attr = pango_attr_rise_new (2 * size / 3);
						pango_attr_list_insert (l, attr);
//						gcp_pango_layout_replace_text (m_Layout, s.index, s.end - s.index + 1, buf, l);
						pango_attr_list_unref (l);
						m_StartSel = m_EndSel = s.index + strlen (buf);
						end += m_StartSel - start - 1;
/*						GnomeCanvasPango* text = pDoc->GetView ()->GetActiveRichText ();
						gnome_canvas_pango_set_selection_bounds (text, m_StartSel, m_EndSel);*/
						g_free (buf);
					}
				}
			}
		} else
			Charge = false;
		start++;
	}
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
	if (!pDoc)
		return NULL;
	double x0, y0;
	x0 = (x - m_x) * pTheme->GetZoomFactor () + m_lbearing;
	y0 = (y - m_y) * pTheme->GetZoomFactor () + m_ascent;
	if ((x0 < 0.) || (x0 > m_length) || (y0 < 0.) || (y0 > m_height))
		return NULL;
	int index, cur, trailing;
	pango_layout_xy_to_index (m_Layout, (int) (x0 * PANGO_SCALE), (int) (y0 * PANGO_SCALE), &index, &trailing);
	char c = m_buf[index];
	cur = index;
	while ((c >= 'a') && (c <= 'z') && cur >=0) {
		cur--;
		c = m_buf[cur];
	}
	if (index - cur > static_cast <int> (Residue::MaxSymbolLength))
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
			residue->SetResidue (r);
		else {
			map<gcu::Atom*, gcu::Bond*>::iterator i;
			Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
			Atom *pOldAtom = m_Atom;
			m_Atom = NULL;
			m_Atom = new FragmentResidue (this, sy);
			m_Atom->SetId ((gchar*) pOldAtom->GetId ());
			if (pBond) {
				pBond->ReplaceAtom (pOldAtom, m_Atom);
				m_Atom->AddBond (pBond);
			}
			delete pOldAtom;
			AddChild (m_Atom);
		}
		m_x -= m_lbearing / pTheme->GetZoomFactor () ;
		PangoRectangle rect;
		pango_layout_index_to_pos (m_Layout, index, &rect);
		m_lbearing = rect.x / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, index + i, &rect);
		m_lbearing += rect.x / PANGO_SCALE;
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
		map<gcu::Atom*, gcu::Bond*>::iterator i;
		Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
		Atom *pOldAtom = m_Atom;
		m_Atom = NULL;
		m_Atom = new FragmentAtom (this, Z);
		m_Atom->SetId ((gchar*) pOldAtom->GetId ());
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
		PangoRectangle rect;
		pango_layout_index_to_pos (m_Layout, index, &rect);
		m_lbearing = rect.x / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, trailing, &rect);
		m_lbearing += rect.x / PANGO_SCALE;
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
	if (m_bLoading)
		return;
	Document *pDoc = (Document*) GetDocument ();
	if (!pDoc) return;
	char const *sym = m_Atom->GetSymbol ();
//	gcp_pango_layout_replace_text (m_Layout, m_BeginAtom, m_EndAtom - m_BeginAtom, sym, pDoc->GetPangoAttrList ());
	m_EndAtom = m_BeginAtom + strlen (sym);
	OnChanged (false);
}

int Fragment::GetElementAtPos (unsigned start, unsigned &end)
{
	int Z;
	char text[4];
	memset (text, 0, 4);
	strncpy (text, ((m_Layout)?pango_layout_get_text ( m_Layout): m_buf.c_str ()) + start, 3);
	for (unsigned i = strlen (text); i > 0; i--) {
		text[i] = 0;
		if ((Z = Element::Z (text))) {
			end = start + i;
			return Z;
		}
	}
	return 0;
}

int Fragment::GetChargePosition (FragmentAtom *pAtom, unsigned char &Pos, double Angle, double &x, double &y)
{
	if ((pAtom != m_Atom) || (m_Atom->GetZ() == 0))
		return 0;
	double width, height;
	Document* pDoc = (Document*) GetDocument ();
	Theme *pTheme = pDoc->GetTheme ();
	if (!pDoc)
		return 0;
	GtkWidget* pWidget = pDoc->GetView ()->GetWidget ();
/*	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
	GnomeCanvasGroup *item = pData->Items[this];
	if (!item)
		return 0;
	GnomeCanvasPango* text = (GnomeCanvasPango*) g_object_get_data (G_OBJECT (item), "fragment");
	if (!GNOME_IS_CANVAS_PANGO (text))
		return 0;
	struct FilterStruct s;
	s.start = 0;
	s.end = m_buf.length ();
	if (m_AttrList == NULL)
		m_AttrList = pango_layout_get_attributes (m_Layout);
	pango_attr_list_filter (m_AttrList, (PangoAttrFilterFunc) filter_func, &s);
	if (s.pal.size () > 0)
		return 0; //localized charges are prohibited if a global charge already exists
	// Get atom bounds
	int result = 0xff;
	PangoRectangle rect;
	pango_layout_index_to_pos (m_Layout, m_BeginAtom, &rect);
	x = rect.x / PANGO_SCALE;
	if (m_BeginAtom != 0)
		result &= 0x6d;
	pango_layout_index_to_pos (m_Layout, m_EndAtom, &rect);
	width = rect.x / PANGO_SCALE - x;
	if (m_EndAtom < m_buf.length ())
		result &= 0xb6;
	width /= pTheme->GetZoomFactor ();
	height = m_height / pTheme->GetZoomFactor ();
	if (m_Atom->GetBondsNumber())
	{
		map<gcu::Atom*, gcu::Bond*>::iterator i;
		Bond* pBond = (Bond*)m_Atom->GetFirstBond(i);
		double angle = pBond->GetAngle2D(m_Atom) + 180.0;
		if ((result & POSITION_NE) && (angle >= 180.0) && (angle <= 270.0)) result -= POSITION_NE;
		if ((result & POSITION_NW) && (((angle >= 270.0) && (angle <= 360.0)) || (fabs(angle) < 0.1))) result -= POSITION_NW;
		if ((result & POSITION_N) && (angle >= 225.0) && (angle <= 315.0)) result -= POSITION_N;
		if ((result & POSITION_SE) && (angle >= 90.0) && (angle <= 180.0)) result -= POSITION_SE;
		if ((result & POSITION_SW) && (((angle >= 0.0) && (angle <= 90.0)) || (fabs(angle - 360.0) < 0.1))) result -= POSITION_SW;
		if ((result & POSITION_S) && (angle >= 45.0) && (angle <= 135.0)) result -= POSITION_S;
		if ((result & POSITION_E) && ((angle <= 225.0) && (angle >= 135.0))) result -= POSITION_E;
		if ((result & POSITION_W) && ((angle >= 315.0) || (angle <= 45.0))) result -= POSITION_W;
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
			return 0;
	} else if (Pos) {
		if (!(Pos & result))
			return 0;
	} else
		return 0;

	switch (Pos) {
	case POSITION_NE:
		x = m_x + width / 2.0;
		y = m_y - height / 2.0;
		return 1;
	case  POSITION_NW:
		x = m_x - width / 2.0;
		y = m_y - height / 2.0;
		return -1;
	case  POSITION_N:
		x = m_x;
		y = m_y - height / 2.0;
		return 2;
	case POSITION_SE:
		x = m_x + width / 2.0;
		y = m_y + height / 2.0;
		return 1;
	case POSITION_SW:
		x = m_x - width / 2.0;
		y = m_y + height / 2.0;
		return -1;
	case POSITION_S:
		x = m_x;
		y = m_y + height / 2.0;
		return -2;
	case POSITION_E:
		x = m_x + width / 2.0;
		y = m_y;
		return 1;
	case POSITION_W:
		x = m_x - width / 2.0;
		y = m_y;
		return -1;
	}*/
	return 0;
}

bool Fragment::Validate ()
{
	char const *charge;
	char *err;
	if ((m_buf.length () == 0)
		&& m_Atom->GetBondsNumber () == 0)
		return true;
	if (m_Atom->GetZ() == 0 || (dynamic_cast <FragmentResidue*> (m_Atom) && !((FragmentResidue*) m_Atom)->GetResidue ())) {
		Document *pDoc = dynamic_cast<Document*> (GetDocument ());
/*		GtkWidget* pWidget = pDoc->GetView ()->GetWidget ();
		WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
		GnomeCanvasGroup *item = pData->Items[this];
		GnomeCanvasPango* text = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (item), "fragment"));
		gnome_canvas_pango_set_selection_bounds (text, m_BeginAtom, (m_EndAtom == m_BeginAtom)? m_EndAtom + 1: m_EndAtom);
		GtkWidget* w = gtk_message_dialog_new (
										GTK_WINDOW (pDoc->GetWindow ()->GetWindow ()),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
										_("Invalid symbol."));
		gtk_dialog_run (GTK_DIALOG (w));
		gtk_widget_destroy (w);*/
		return false;
	}
	//now scan for charges and validate
	struct FilterStruct s;
	s.start = 0;
	s.end = m_buf.length ();
	if (m_AttrList == NULL)
		m_AttrList = pango_layout_get_attributes (m_Layout);
	pango_attr_list_filter (m_AttrList, (PangoAttrFilterFunc) filter_func, &s);
	list<PangoAttribute*>::iterator i, iend = s.pal.end ();
	for (i = s.pal.begin (); i != iend; i++) {
		charge = m_buf.c_str () + (*i)->start_index;
		strtol (charge, &err, 10);
		if (*err != '+' && *err != '-' && err - m_buf.c_str () != (int) (*i)->end_index) {
			Document *pDoc = dynamic_cast<Document*> (GetDocument ());
/*			GtkWidget* pWidget = pDoc->GetView ()->GetWidget ();
			WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (pWidget), "data");
			GnomeCanvasGroup *item = pData->Items[this];
			GnomeCanvasPango* text = GNOME_CANVAS_PANGO (g_object_get_data (G_OBJECT (item), "fragment"));
			gnome_canvas_pango_set_selection_bounds (text, (*i)->start_index, (*i)->end_index);
			GtkWidget* w = gtk_message_dialog_new (
											GTK_WINDOW (pDoc->GetWindow ()->GetWindow ()),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
											_("Invalid charge."));
			gtk_dialog_run(GTK_DIALOG(w));
			gtk_widget_destroy(w);*/
			return false;
		}
	}
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
	
double Fragment::GetYAlign ()
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
		sscanf (value, "%lg %lg", &m_x, &m_y);
		gcu::Document *doc = GetDocument ();
		if (doc) {
			m_x *= doc->GetScale ();
			m_y *= doc->GetScale ();
		}
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
				map<gcu::Atom*, gcu::Bond*>::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				pOldAtom->SetParent (NULL);
				m_Atom = new FragmentResidue (this, sy);
				AddChild (m_Atom);
				m_Atom->SetId ((gchar*) pOldAtom->GetId ());
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
				map<gcu::Atom*, gcu::Bond*>::iterator i;
				Bond *pBond = (gcp::Bond*) m_Atom->GetFirstBond (i);
				Atom *pOldAtom = m_Atom;
				pOldAtom->SetParent (NULL);
				m_Atom = new FragmentResidue (this, sy);
				AddChild (m_Atom);
				m_Atom->SetId ((gchar*) pOldAtom->GetId ());
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

bool Fragment::Analyze () {
	// search if main atom is at start or at end
	if ((m_BeginAtom == 0 || m_EndAtom == m_buf.size ()) && ((m_EndAtom - m_BeginAtom) < m_buf.length ()) ){
		m_Inversable = true;
	}
//	int valence = m_Atom->GetValence ();
	if (m_AttrList != NULL)
		pango_attr_list_unref (m_AttrList);
	m_AttrList = pango_attr_list_new ();
	AnalContent ();
	return true;
}

void Fragment::Update () {
	if (m_Atom->GetBondsNumber () > 0 && m_Inversable) {
		map<gcu::Atom*, gcu::Bond*>::iterator i;
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
			if (m_AttrList != NULL)
				pango_attr_list_unref (m_AttrList);
			m_AttrList = pango_attr_list_new ();
			AnalContent ();
		} else if (angle > 91. || angle < -91.) {
		}
	}
}

}	//	namespace gcp
