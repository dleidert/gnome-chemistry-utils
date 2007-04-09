// -*- C++ -*-

/* 
 * GChemPaint library
 * atom.cc
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "atom.h"
#include "electron.h"
#include "bond.h"
#include "molecule.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "document.h"
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-rect-ellipse.h>
#include <canvas/gcp-canvas-bpath.h>
#include <canvas/gcp-canvas-pango.h>
#include <gcu/element.h>
#include <openbabel/mol.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;

#define ATOM_EPSILON 0.1

namespace gcp {

Atom::Atom (): gcu::Atom (),
	m_ShowSymbol (false)
{
	m_Valence = -1; //unspecified
	m_nlp = 0;
	m_nH = 0;
	m_HPos = GetBestSide();
	m_ChargeAuto = false;
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = NULL;
	m_DrawCircle = false;
}

Atom::~Atom ()
{
	Document *pDoc = (Document*) GetDocument ();
	if (!pDoc)
		return;
	View *pView = pDoc->GetView ();
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron) {
		pView->Remove (electron);
		electron->SetParent (NULL); // avoids a call to RemoveElectron()
		delete electron;
		electron = GetNextChild (i);
	}
	if (m_Layout)
		g_object_unref (G_OBJECT (m_Layout));
	if (m_ChargeLayout)
		g_object_unref (G_OBJECT (m_ChargeLayout));
}

Atom::Atom (int Z, double x, double y, double z): gcu::Atom (Z, x, y, z),
	m_ShowSymbol (false)
{
	m_ChargeAuto = false;
	m_HPos = GetBestSide ();
	m_nlp = 0;
	SetZ(Z);
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = NULL;
	m_DrawCircle = false;
}

Atom::Atom (OBAtom* atom): gcu::Atom (),
	m_ShowSymbol (false)
{
	m_x = atom->GetX ();
	m_y = - atom->GetY ();
	m_z = atom->GetZ ();
	m_nlp = 0;
	SetZ (atom->GetAtomicNum ());
	gchar* Id = g_strdup_printf ("a%d", atom->GetIdx());
	SetId (Id);
	g_free (Id);
	m_HPos = true;
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = NULL;
	m_DrawCircle = false;
	m_Charge = atom->GetFormalCharge ();
}

void Atom::SetZ (int Z)
{
	gcu::Atom::SetZ (Z);
	m_Element = Element::GetElement (m_Z);
	if ((m_Valence = m_Element->GetDefaultValence ()))
		m_HPos = GetBestSide ();
	else
		m_nH = 0;
	int max = m_Element->GetMaxValenceElectrons ();
	int diff = m_Element->GetTotalValenceElectrons () - m_Element->GetValenceElectrons ();
	switch (max) {
	case 2:
		m_ValenceOrbitals = 1;
		break;
	case 8:
		m_ValenceOrbitals = 4;
		break;
	case 18:
		if (!diff)
			m_ValenceOrbitals = 6;
		else
			m_ValenceOrbitals = 4;
		break;
	case 32:
		if (!diff)
			m_ValenceOrbitals = 8;
		else if (diff == 14)
			m_ValenceOrbitals = 6;
		else
			m_ValenceOrbitals = 4;
		break;
	default:
		m_ValenceOrbitals = 0; //should not occur
	}
	Update();
	EmitSignal (OnChangedSignal);
}

int Atom::GetTotalBondsNumber ()
{
	std::map<gcu::Atom*, gcu::Bond*>::iterator i, end = m_Bonds.end ();
	int n = 0;
	for (i = m_Bonds.begin(); i != end; i++)
		n += (*i).second->GetOrder ();
	return n;
}

void Atom::AddBond (gcu::Bond* pBond)
{
	gcu::Atom::AddBond (pBond);
	Update ();
}

void Atom::RemoveBond (gcu::Bond* pBond)
{
	gcu::Atom::RemoveBond (pBond);
	Update ();
}

bool Atom::GetBestSide ()
{
	if (m_Bonds.size () == 0)
		return Element::BestSide (m_Z);
	std::map<gcu::Atom*, gcu::Bond*>::iterator i, end = m_Bonds.end();
	double sum = 0.0;
	for (i = m_Bonds.begin(); i != end; i++)
		sum -= cos(((Bond*) (*i).second)->GetAngle2DRad (this));
	if (fabs(sum) > 0.1)
		return (sum >= 0.0);
	else
		return Element::BestSide (m_Z);
}

void Atom::Update ()
{
	if (m_ChargeAuto) {
		m_Charge = 0;
		m_ChargeAuto = false;
	}
	if (m_ChargeAutoPos) {
		NotifyPositionOccupation (m_ChargePos, false);
		m_ChargePos = 0xff;
	}
	int nb, nexplp = 0, nexplu = 0; //nexplp is the number of explicit lone pairs
	//nexplu is the number of explicit unpaired electrons
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	while (electron) { 
		if (electron->IsPair ())
			nexplp++;
		else
			nexplu++;
		electron = (Electron*) GetNextChild (i);
	}
	int nbonds = GetTotalBondsNumber ();
	if (m_Valence > 0) {
		m_nlp = (m_Element->GetValenceElectrons () - m_Valence) / 2;
		if ((m_Charge > 0) && (m_nlp > 0)) m_nlp -= (m_Charge + 1) / 2;
		else if (m_Charge < 0)
			m_nlp -= m_Charge;
		if (m_nlp < nexplp) // Can this occur ?
			m_nlp = nexplp;
		else if (m_nlp > m_ValenceOrbitals - nbonds - nexplu)
			m_nlp = m_ValenceOrbitals - nbonds - nexplu;
		if (m_nlp < 0)
			m_nlp = 0;
		nb = m_Element->GetValenceElectrons () - 2 * m_nlp - m_Charge;
		if (nb + m_nlp > 4) nb -= 2; //octet rule
		m_nH = nb - nbonds - nexplu;
		if (!m_Charge && m_nH == -1 && m_nlp > 0)
		{
			m_Charge = m_Element->GetValenceElectrons () - nbonds
						- m_nlp * 2 - nexplu;
			m_ChargeAuto = true;
			m_nH = 0;
		}
		if (m_nH < 0) { // extended octet or missing core electrons
			m_nH = 0;
			if (m_nlp || nexplu || nbonds) {
				m_Charge = m_Element->GetValenceElectrons () - 2 * m_nlp - nexplu - nbonds;
				m_ChargeAuto = true;
			}
		}
		m_HPos = GetBestSide ();
	} else {
		m_nH = 0;
		if (m_ChargeAuto || !m_Charge) {
			m_Charge = m_Element->GetValenceElectrons () - 2 * nexplp - nexplu - nbonds;
			if (m_Charge > 0)
				m_Charge = 0;
			m_ChargeAuto = true;
		}
	}
	Document *pDoc = (Document *) GetDocument ();
	if (pDoc)
		m_Changed = pDoc->GetView ()->GetNbWidgets ();
	m_AvailPosCached = false;
	if (nbonds && GetZ () == 6) {
		// update large bonds ends
		Bond *bond;
		BondType type;
		bool DrawCircle;
		map<gcu::Atom*, gcu::Bond*>::iterator i = m_Bonds.begin(), iend = m_Bonds.end ();
		int nb = 0;
		while (i != iend)
		{
			bond = dynamic_cast<Bond*> ((Bond*)(*i).second);
			type = bond->GetType ();
			if (type == ForeBondType || (type == UpBondType && bond->GetAtom (1) == this))
				nb++;
			i++;
		}
		DrawCircle = nb > 1;
		if (!DrawCircle && GetBondsNumber () == 2) {
			i = m_Bonds.begin();
			double angle = static_cast<Bond*> ((*i).second)->GetAngle2D (this);
			i++;
			angle -= static_cast<Bond*> ((*i).second)->GetAngle2D (this);
			while (angle < 0)
				angle += 360.;
			while (angle > 360.)
				angle -= 360;
			if (fabs (angle - 180.) < 1)
				DrawCircle = true;
		}
		if (DrawCircle != m_DrawCircle) {
			m_DrawCircle = DrawCircle;
			m_Changed = true;
		}
	}
}
	
bool Atom::IsInCycle (Cycle* pCycle)
{
	map<gcu::Atom*, gcu::Bond*>::iterator i, end = m_Bonds.end ();
	for (i = m_Bonds.begin (); i != end; i++)
		if (((Bond*) (*i).second)->IsInCycle (pCycle))
		return true;
	return false;
}

void Atom::Add (GtkWidget* w)
{
	if (!w)
		return;
	if (m_Changed > 0)
		m_Changed--;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	View* pView = pData->m_View;
	Theme *pTheme = pView->GetDoc ()->GetTheme ();
	if (m_Layout == NULL) {
		PangoContext* pc = pView->GetPangoContext ();
		m_Layout = pango_layout_new (pc);
	}
	if (m_FontName != pView->GetFontName ()) {
		pango_layout_set_font_description (m_Layout, pView->GetPangoFontDesc ());
		pango_layout_set_text (m_Layout, "l", 1);
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		m_FontName = pView->GetFontName ();
		m_CHeight = 0.;
	}
	PangoRectangle rect;
	if (m_CHeight == 0.) {
		pango_layout_set_text (m_Layout, "C", 1);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
	}
	double x, y, xc = 0., yc;
	m_width =  m_height = 2.0 * pTheme->GetPadding ();
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group, *chgp;
	group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	g_signal_connect (G_OBJECT (group), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "object", this);
	if ((GetZ () != 6) || (GetBondsNumber () == 0)) {
		int sw, sp;
		const gchar* symbol = GetSymbol (), *text;
		sw = strlen (symbol);
		pango_layout_set_text (m_Layout, symbol, sw);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		m_width += rect.width / PANGO_SCALE;
		int n = GetAttachedHydrogens ();
		if (n > 0) {
			if (n > 1) {
				gchar const *nb =  g_strdup_printf ("%d", n);
				int np, nw = strlen (nb);
				if (m_HPos) {
					text = g_strconcat (symbol, "H", nb, NULL);
					np = sw + 1;
					sp = 0;
				} else {
					text = g_strconcat ("H", nb, symbol, NULL);
					np = 1;
					sp = np + nw;
				}
				pango_layout_set_text (m_Layout, text, -1);
				PangoAttrList *pal = pango_attr_list_new ();
				PangoAttribute *attr = pango_attr_font_desc_new (pView->GetPangoSmallFontDesc());
				attr->start_index = np;
				attr->end_index = np + nw;
				pango_attr_list_insert (pal, attr);
				attr = pango_attr_rise_new (-2 * PANGO_SCALE);
				attr->start_index = np;
				attr->end_index = np + nw;
				pango_attr_list_insert (pal, attr);
				pango_layout_set_attributes (m_Layout, pal);
				pango_attr_list_unref (pal);
			} else {
				if (m_HPos) {
					text = g_strconcat (symbol, "H", NULL);
					sp = 0;
				} else {
					text = g_strconcat ("H", symbol, NULL);
					sp = 1;
				}
				pango_layout_set_text (m_Layout, text, -1);
			}
		} else {
			text = g_strdup (symbol);
			sp = 0;
			pango_layout_set_text (m_Layout, text, -1);
		}
		pango_layout_get_extents (m_Layout, NULL, &rect);
		m_length = double (rect.width / PANGO_SCALE);
		m_text_height = m_height = rect.height / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, sp, &rect);
		int st = rect.x / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, sp + sw, &rect);
		m_lbearing = (st + rect.x / PANGO_SCALE) / 2.;

		item = gnome_canvas_item_new (
							group,
							gnome_canvas_rect_ext_get_type (),
							"x1", x - m_lbearing - pTheme->GetPadding (),
							"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding (),
							"x2", x - m_lbearing + m_length + pTheme->GetPadding (),
							"y2", y  - m_ascent + m_CHeight + m_height + pTheme->GetPadding (),
							"fill_color", (pData->IsSelected (this))? SelectColor: "white",
							NULL);
		g_object_set_data (G_OBJECT (group), "rect", item);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		g_object_set_data (G_OBJECT (item), "object", this);
		
		item = gnome_canvas_item_new (
							group,
							gnome_canvas_pango_get_type (),
							"x", x - m_lbearing,
							"y", y - m_ascent + m_CHeight,
							"layout", m_Layout,
						NULL);
		g_object_set_data (G_OBJECT (group), "symbol", item);
		g_object_set_data (G_OBJECT (item), "object", this);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	} else {
		item = gnome_canvas_item_new (
								group,
								gnome_canvas_rect_ext_get_type (),
								"x1", x - 3,
								"y1", y - 3,
								"x2", x + 3,
								"y2", y + 3,
								"fill_color",  (pData->IsSelected (this))? SelectColor: "white",
								NULL);
		g_object_set_data(G_OBJECT (group), "rect", item);
		gnome_canvas_request_redraw ((GnomeCanvas*) w, (int) x - 3, (int) y - 3, (int) x + 3, (int) y + 3);
		gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM (group));
		gnome_canvas_item_raise (GNOME_CANVAS_ITEM (group), 1);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		g_object_set_data (G_OBJECT (item), "object", this);
		if (m_DrawCircle) {
			double dx = pTheme->GetStereoBondWidth () / 2.;
			item = gnome_canvas_item_new (
									group,
									gnome_canvas_ellipse_ext_get_type (),
									"x1", x - dx,
									"y1", y - dx,
									"x2", x + dx,
									"y2", y + dx,
									"fill_color",  (pData->IsSelected (this))? SelectColor: Color,
									NULL);
			g_object_set_data (G_OBJECT (group), "bullet", item);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
			g_object_set_data (G_OBJECT (item), "object", this);
		}
	}
	pData->Items[this] = group;
	m_width /= pTheme->GetZoomFactor ();
	m_height /= pTheme->GetZoomFactor ();
	/* add charge */
	int charge = GetCharge ();
	if (charge) {
		int align = GetChargePosition (m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
		if (m_ChargeDist != 0.) {
			align = 0;
			x = m_x + m_ChargeDist * cos (m_ChargeAngle);
			y = m_y - m_ChargeDist * sin (m_ChargeAngle);
		}
		x *= pTheme->GetZoomFactor ();
		y *= pTheme->GetZoomFactor ();
		char *fig = NULL;
		if 	(abs (charge) > 1) {
			fig = g_strdup_printf ("%d", abs (charge));
			PangoRectangle rect;
			if (!m_ChargeLayout) {
				PangoContext* pc = pData->m_View->GetPangoContext();
				m_ChargeLayout = pango_layout_new (pc);
				pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
			}
			pango_layout_set_text (m_ChargeLayout, fig, -1);
			pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
			m_ChargeWidth = rect.width / PANGO_SCALE;
			m_ChargeTWidth = m_ChargeWidth + 1. + pTheme->GetChargeSignSize ();
		} else {
			m_ChargeWidth = 0.;
			m_ChargeTWidth = pTheme->GetChargeSignSize ();
		}
		switch (align) {
		case -2:
			xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
			y += pTheme->GetChargeSignSize () / 2.;
			break;
		case -1:
			xc = x - pTheme->GetChargeSignSize () - pTheme->GetPadding ();
			break;
		case 0:
		case -3:
			xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
			break;
		case 1:
			xc = x + m_ChargeWidth + pTheme->GetPadding ();
			break;
		case 2:
			xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
			y -= pTheme->GetChargeSignSize () / 2.;
			break;
		}
		x = xc - 1.;
		yc = y - pTheme->GetChargeSignSize () / 2.;
		chgp = (GnomeCanvasGroup*) gnome_canvas_item_new (
					group,
					gnome_canvas_group_ext_get_type(),
					NULL);
		g_object_set_data (G_OBJECT (group), "charge", chgp);
		if (fig) {
			item = gnome_canvas_item_new(
						chgp,
						gnome_canvas_pango_get_type(),
						"layout", m_ChargeLayout,
						"fill_color", (pData->IsSelected(this))? SelectColor: Color,
						"anchor", GTK_ANCHOR_EAST,
						"x", x,
						"y", y,
						NULL);
			g_object_set_data (G_OBJECT (group), "figure", item);
			g_free (fig);
		}
		item = gnome_canvas_item_new (
					chgp,
					gnome_canvas_ellipse_ext_get_type (),
					"x1", xc,
					"y1", yc,
					"x2", xc + pTheme->GetChargeSignSize (),
					"y2", yc + pTheme->GetChargeSignSize (),
					"outline_color", (pData->IsSelected(this))? SelectColor: Color,
					"width_units", 0.5,
					NULL
				);
		g_object_set_data (G_OBJECT (group), "circle", item);
		ArtBpath *path = art_new (ArtBpath, 5);
		path[0].code = ART_MOVETO_OPEN;
		path[0].x3 = xc + 1.;
		path[1].code = ART_LINETO;
		path[1].x3 = xc + pTheme->GetChargeSignSize () - 1.;
		path[0].y3 = path[1].y3 = yc + pTheme->GetChargeSignSize () / 2.;
		if (charge > 0) {
			path[2].code = ART_MOVETO_OPEN;
			path[2].y3 = yc + 1.;
			path[3].code = ART_LINETO;
			path[3].y3 = yc + pTheme->GetChargeSignSize () - 1.;
			path[2].x3 = path[3].x3 = xc + pTheme->GetChargeSignSize () / 2.;
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
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron){
		electron->Add (w);
		electron = GetNextChild (i);
	}
}

void Atom::Update (GtkWidget* w)
{
	if (!w)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y, xc = 0., yc;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	GnomeCanvasGroup *group = pData->Items[this];
	if (m_FontName != pData->m_View->GetFontName ()) {
		View *pView = pData->m_View;
		PangoContext* pc = pView->GetPangoContext ();
		PangoLayout *Layout = pango_layout_new (pc);
		pango_layout_set_font_description (Layout, pView->GetPangoFontDesc ());
		pango_layout_set_font_description (m_Layout, pView->GetPangoFontDesc ());
		pango_layout_set_text (Layout, "l", 1);
		PangoLayoutIter* iter = pango_layout_get_iter (Layout);
		m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		m_FontName = pView->GetFontName ();
		pango_layout_set_text (Layout, "C", 1);
		PangoRectangle rect;
		pango_layout_get_extents (Layout, &rect, NULL);
		m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
		g_object_unref (G_OBJECT (Layout));
	}
	if (m_Changed)
		BuildItems (pData);
	else {
		if ((GetZ() != 6) || (GetBondsNumber () == 0) || m_ShowSymbol) {
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "symbol")),
								"x", x - m_lbearing,
								"y", y - m_ascent + m_CHeight,
								NULL);
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "rect")),
								"x1", x - m_lbearing - pTheme->GetPadding (),
								"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding (),
								"x2", x - m_lbearing + m_length + pTheme->GetPadding (),
								"y2", y  - m_ascent + m_CHeight + m_text_height + pTheme->GetPadding (),
								NULL);
		} else {
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "rect")),
									"x1", x - 3,
									"y1", y - 3,
									"x2", x + 3,
									"y2", y + 3,
								NULL);
			if (m_DrawCircle) {
				double dx = pTheme->GetStereoBondWidth () / 2.;
				g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "bullet")),
										"x1", x - dx,
										"y1", y - dx,
										"x2", x + dx,
										"y2", y + dx,
										NULL);
			}
		}
	}
	void* item = g_object_get_data (G_OBJECT (group), "charge");
	int charge = GetCharge ();
	if (charge) {
		if (item) {
			int align = GetChargePosition (m_ChargePos, m_ChargeAngle * 180. / M_PI, x, y);
			if (m_ChargeDist != 0.) {
				align = 0;
				x = m_x + m_ChargeDist * cos (m_ChargeAngle);
				y = m_y - m_ChargeDist * sin (m_ChargeAngle);
			}
			x *= pTheme->GetZoomFactor ();
			y *= pTheme->GetZoomFactor ();
			GnomeCanvasItem *figure = (GnomeCanvasItem*) g_object_get_data (G_OBJECT (group), "figure");
			char *fig = NULL;
			if 	(abs (charge) > 1) {
				fig = g_strdup_printf ("%d", abs (charge));
				PangoRectangle rect;
				if (!m_ChargeLayout) {
					PangoContext* pc = pData->m_View->GetPangoContext ();
					m_ChargeLayout = pango_layout_new (pc);
					pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
				}
				pango_layout_set_text (m_ChargeLayout, fig, -1);
				pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
				m_ChargeWidth = rect.width / PANGO_SCALE;
			} else
				m_ChargeWidth = 0.;
			m_ChargeTWidth = m_ChargeWidth + 1. + pTheme->GetChargeSignSize ();
			if (figure == NULL && fig != NULL) {
				figure = gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (item),
							gnome_canvas_pango_get_type (),
							"anchor", GTK_ANCHOR_EAST,
							NULL);
				g_object_set_data (G_OBJECT (group), "figure", figure);
			} else if (figure != NULL && fig == NULL) {
				gtk_object_destroy (GTK_OBJECT (figure));
				g_object_set_data ((GObject*)group, "figure", NULL);
			}
			switch (align) {
			case -2:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				y += pTheme->GetChargeSignSize () / 2.;
				break;
			case -1:
				xc = x - pTheme->GetChargeSignSize () - pTheme->GetPadding ();
				break;
			case 0:
			case -3:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				break;
			case 1:
				xc = x + m_ChargeWidth + pTheme->GetPadding ();
				break;
			case 2:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				y -= pTheme->GetChargeSignSize () / 2.;
				break;
			}
			x = xc -1.;
			yc = y - pTheme->GetChargeSignSize () / 2.;
			if (fig) {
				g_object_set (G_OBJECT (figure),
							"layout", m_ChargeLayout,
							"x", x,
							"y", y,
							NULL);
				g_free (fig);
			}
			item = g_object_get_data (G_OBJECT (group), "circle");
			g_object_set (G_OBJECT (item),
						"x1", xc,
						"y1", yc,
						"x2", xc + pTheme->GetChargeSignSize (),
						"y2", yc + pTheme->GetChargeSignSize (),
						NULL);
			item = g_object_get_data (G_OBJECT (group), "sign");
			ArtBpath *path = art_new (ArtBpath, 5);
			path[0].code = ART_MOVETO_OPEN;
			path[0].x3 = xc + 1.;
			path[1].code = ART_LINETO;
			path[1].x3 = xc + pTheme->GetChargeSignSize () - 1.;
			path[0].y3 = path[1].y3 = yc + pTheme->GetChargeSignSize () / 2.;
			if (charge > 0) {
				path[2].code = ART_MOVETO_OPEN;
				path[2].y3 = yc + 1.;
				path[3].code = ART_LINETO;
				path[3].y3 = yc + pTheme->GetChargeSignSize () - 1.;
				path[2].x3 = path[3].x3 = xc + pTheme->GetChargeSignSize () / 2.;
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
			int align = GetChargePosition(m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
			if (m_ChargeDist != 0.) {
				align = 0;
				x = (m_x + m_ChargeDist * cos (m_ChargeAngle));
				y = (m_y - m_ChargeDist * sin (m_ChargeAngle));
			}
			x *= pTheme->GetZoomFactor ();
			y *= pTheme->GetZoomFactor ();

			char *fig = NULL;
			if 	(abs (charge) > 1) {
				fig = g_strdup_printf ("%d", abs (charge));
				if (!m_ChargeLayout) {
					PangoContext* pc = pData->m_View->GetPangoContext ();
					m_ChargeLayout = pango_layout_new (pc);
					pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
				}
				pango_layout_set_text (m_ChargeLayout, fig, -1);
				PangoRectangle rect;
				pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
				m_ChargeWidth = rect.width / PANGO_SCALE;
				m_ChargeTWidth = m_ChargeWidth + pTheme->GetPadding () + pTheme->GetChargeSignSize ();
			} else {
				m_ChargeWidth = 0.;
				m_ChargeTWidth = pTheme->GetChargeSignSize ();
			}
			switch (align) {
			case -2:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				y += pTheme->GetChargeSignSize () / 2.;
				break;
			case -1:
				xc = x - pTheme->GetChargeSignSize () - pTheme->GetPadding ();
				break;
			case 0:
			case -3:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				break;
			case 1:
				xc = x + m_ChargeWidth + pTheme->GetPadding ();
				break;
			case 2:
				xc = x + m_ChargeTWidth / 2. - pTheme->GetChargeSignSize ();
				y -= pTheme->GetChargeSignSize () / 2.;
				break;
			}
			x = xc - 1.;
			yc = y - pTheme->GetChargeSignSize () / 2.;
			chgp = (GnomeCanvasGroup*) gnome_canvas_item_new (
						group,
						gnome_canvas_group_ext_get_type (),
						NULL);
			g_object_set_data (G_OBJECT (group), "charge", chgp);
			if (fig) {
				item = gnome_canvas_item_new (
							chgp,
							gnome_canvas_pango_get_type (),
							"layout", m_ChargeLayout,
							"fill_color", (pData->IsSelected (this))? SelectColor: Color,
							"anchor", GTK_ANCHOR_EAST,
							"x", x,
							"y", y,
							NULL);
				g_object_set_data (G_OBJECT (group), "figure", item);
				g_free (fig);
			}
			item = gnome_canvas_item_new (
						chgp,
						gnome_canvas_ellipse_ext_get_type (),
						"x1", xc,
						"y1", yc,
						"x2", xc + pTheme->GetChargeSignSize (),
						"y2", yc + pTheme->GetChargeSignSize (),
						"outline_color", (pData->IsSelected (this))? SelectColor: Color,
						"width_units", .5,
						NULL
					);
			g_object_set_data (G_OBJECT (group), "circle", item);
			ArtBpath *path = art_new (ArtBpath, 5);
			path[0].code = ART_MOVETO_OPEN;
			path[0].x3 = xc + 1.;
			path[1].code = ART_LINETO;
			path[1].x3 = xc + pTheme->GetChargeSignSize () - 1.;
			path[0].y3 = path[1].y3 = yc + pTheme->GetChargeSignSize () / 2.;
			if (charge > 0) {
				path[2].code = ART_MOVETO_OPEN;
				path[2].y3 = yc + 1.;
				path[3].code = ART_LINETO;
				path[3].y3 = yc + pTheme->GetChargeSignSize () - 1.;
				path[2].x3 = path[3].x3 = xc + pTheme->GetChargeSignSize () / 2.;
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
		if (item)
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data ((GObject*)group, "charge", NULL);
			g_object_set_data ((GObject*)group, "figure", NULL);
			g_object_set_data ((GObject*)group, "circle", NULL);
			g_object_set_data ((GObject*)group, "sign", NULL);
	}
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron){
		electron->Update (w);
		electron = GetNextChild (i);
	}
}

void Atom::UpdateAvailablePositions ()
{
	list<double>::iterator n;
	double angle, delta, dir;
	m_AngleList.clear ();
	if (((GetZ() != 6 || m_Bonds.size() == 0)) && m_nH) {
		if (m_HPos) {
			m_AvailPos = 0xB6;
			m_AngleList.push_front(315.0);
			m_AngleList.push_front(45.0);
		} else {
			m_AvailPos = 0x6D;
			m_AngleList.push_front(225.0);
			m_AngleList.push_front(135.0);
		}
	} else
		m_AvailPos = 0xff;
	m_AvailPos &= ~m_OccupiedPos;
	map<gcu::Atom*, gcu::Bond*>::iterator i = m_Bonds.begin();
	while (i != m_Bonds.end()) {
		n = m_AngleList.begin ();
		angle = ((Bond*) (*i).second)->GetAngle2D (this);
		if (angle < 0)
			angle += 360.;
		while ((n != m_AngleList.end ()) && (*n < angle)) n++;
		m_AngleList.insert (n, angle);
		i++;
		if ((m_AvailPos & CHARGE_SW) && (angle >= 180.0 - ATOM_EPSILON) &&
			(angle <= 270.0 + ATOM_EPSILON))
			m_AvailPos -= CHARGE_SW;
		if ((m_AvailPos & CHARGE_SE) && (((angle >= 270.0 - ATOM_EPSILON) &&
			(angle <= 360.0 + ATOM_EPSILON)) || (fabs(angle) < ATOM_EPSILON)))
			m_AvailPos -= CHARGE_SE;
		if ((m_AvailPos & CHARGE_S) && (angle >= 225.0 - ATOM_EPSILON) &&
			(angle <= 315.0 + ATOM_EPSILON))
			m_AvailPos -= CHARGE_S;
		if ((m_AvailPos & CHARGE_NW) && (angle >= 90.0 - ATOM_EPSILON) &&
			(angle <= 180.0 + ATOM_EPSILON))
			m_AvailPos -= CHARGE_NW;
		if ((m_AvailPos & CHARGE_NE) && (((angle >= 0.0 - ATOM_EPSILON) &&
			(angle <= 90.0 + ATOM_EPSILON)) || (fabs(angle - 360.0) < ATOM_EPSILON)))
			m_AvailPos -= CHARGE_NE;
		if ((m_AvailPos & CHARGE_N) && (angle >= 45.0 - ATOM_EPSILON) &&
			(angle <= 135.0 + ATOM_EPSILON))
			m_AvailPos -= CHARGE_N;
		if ((m_AvailPos & CHARGE_W) && ((angle <= 225.0 + ATOM_EPSILON) &&
			(angle >= 135.0 - ATOM_EPSILON)))
			m_AvailPos -= CHARGE_W;
		if ((m_AvailPos & CHARGE_E) && ((angle >= 315.0 - ATOM_EPSILON) ||
			(angle <= 45.0 + ATOM_EPSILON)))
			m_AvailPos -= CHARGE_E;
	}
	m_AngleList.push_back ((angle = m_AngleList.front ()) + 360.0);
	m_InterBonds.clear ();
	for (n = m_AngleList.begin (), n++; n != m_AngleList.end (); n++) {
		delta = *n - angle;
		while (m_InterBonds.find (delta) != m_InterBonds.end ())
			delta -= 1e-8;
		dir = (*n + angle) / 2.;
		if ((m_AvailPos == 0xff) || (m_HPos && (dir < 135. || dir > 225.)) ||
			(!m_HPos && (dir > 45. && dir < 315.)))
			m_InterBonds[delta] = dir;
		angle = *n;
	}
	m_AvailPosCached = true;
}

int Atom::GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y)
{
	list<double>::iterator n, end;
	double angle;
	if (m_ChargePos != 0xff)
		m_OccupiedPos &= ~m_ChargePos;
	if (!m_AvailPosCached)
		UpdateAvailablePositions ();
	if (m_ChargePos != 0xff)
		m_OccupiedPos |= m_ChargePos;
	if (!m_ChargeAutoPos && Pos == 0xff) {
		Pos = m_ChargePos;
		if (!Pos)
			Angle = m_ChargeAngle * 180 / M_PI;
	} else if (Pos == 0xff) {
		if (m_AvailPos) {
			if (m_AvailPos & CHARGE_NE)
				Pos = CHARGE_NE;
			else if (m_AvailPos & CHARGE_NW)
				Pos = CHARGE_NW;
			else if (m_AvailPos & CHARGE_N)
				Pos = CHARGE_N;
			else if (m_AvailPos & CHARGE_SE)
				Pos = CHARGE_SE;
			else if (m_AvailPos & CHARGE_SW)
				Pos = CHARGE_SW;
			else if (m_AvailPos & CHARGE_S)
				Pos = CHARGE_S;
			else if (m_AvailPos & CHARGE_E)
				Pos = CHARGE_E;
			else if (m_AvailPos & CHARGE_W)
				Pos = CHARGE_W;
		} else {
			Pos = 0;
			angle = m_AngleList.front();
			double max = 0.0;
			n = m_AngleList.end ();
			//if we are there, there are at least two bonds
			for (n = m_AngleList.begin (), n++; n != end; n++) {
				if (*n - angle > max) {
					if (*n - angle - max > 0.1) x = (*n + angle) / 2;
					if (m_nH) {
						if (m_HPos && ((x > 225.0) || (x < 135.0)))
							Angle = x;
						else if (m_HPos && (x > 45.0) && (x < 315.0))
							Angle = x;
					}
					else Angle = x;
					max = *n - angle;
				}
				angle = *n;
			}
		} 
	} else if (Pos) {
		if (!(Pos & m_AvailPos) && (Pos != m_ChargePos))
			return 0;
	} else {
		if (Angle > 360.)
			Angle -= 360;
		else if (Angle < 0.)
			Angle += 360;
		if (!(((GetZ() == 6) && (m_Bonds.size() != 0)) ||
			 !m_nH || ((!m_HPos && (Angle < 135. || Angle > 225.)) ||
				(m_HPos && (Angle > 45. && Angle < 315.)))))
			return 0;
	}
	switch (Pos) {
	case CHARGE_NE:
		x = m_x + m_width / 2.0;
		y = m_y - m_height / 2.0;
		return 1;
	case CHARGE_NW:
		x = m_x - m_width / 2.0;
		y = m_y - m_height / 2.0;
		return -1;
	case CHARGE_N:
		x = m_x;
		y = m_y - m_height / 2.0;
		return 2;
	case CHARGE_SE:
		x = m_x + m_width / 2.0;
		y = m_y + m_height / 2.0;
		return 1;
	case CHARGE_SW:
		x = m_x - m_width / 2.0;
		y = m_y + m_height / 2.0;
		return -1;
	case CHARGE_S:
		x = m_x;
		y = m_y + m_height / 2.0;
		return -2;
	case CHARGE_E:
		x = m_x /*+ 12.*/ + m_width / 2.0;
		y = m_y;
		return 1;
	case CHARGE_W:
		x = m_x /*- 12.*/ - m_width / 2.0;
		y = m_y;
		return -1;
	default: {
			double t = tan (Angle / 180. * M_PI);
			double limit = atan (m_height / m_width) * 180. / M_PI;
			if (Angle < limit) {
				x = m_x /*+  12. */+ m_width / 2.;
				y = m_y - m_width / 2. * t;
				return 1;
			} else if (Angle < 180. - limit) {
				if (!isnan (t))
					x = m_x + m_height / 2. / t;
				else
					x = m_x;
				y = m_y - m_height / 2.;
				return 2;
			} else if (Angle < 180. + limit) {
				x = m_x /*- 12.*/ - m_width / 2.;
				y = m_y + m_width / 2. * t;
				return -1;
			} else if (Angle < 360. - limit) {
				if (!isnan (t))
					x = m_x - m_height / 2. / t;
				else
					x = m_x;
				y = m_y + m_height / 2.;
				return -2;
			} else {
				x = m_x /*+  12.*/ + m_width / 2.;
				y = m_y - m_width / 2. * t;
				return 1;
			}
		}			
	}
	return 0; // should not occur
}

int Atom::GetAvailablePosition (double& x, double& y)
{
	list<double>::iterator n, end;
	double angle;
	if (!m_AvailPosCached)
		UpdateAvailablePositions ();
	if (m_AvailPos) {
		if (m_AvailPos & POSITION_N) {
			x = m_x;
			y = m_y - m_height / 2.0;
			return POSITION_N;
		}
		if (m_AvailPos & POSITION_S) {
			x = m_x;
			y = m_y + m_height / 2.0;
			return POSITION_S;
		}
		if (m_AvailPos & POSITION_E) {
			x = m_x + m_width / 2.0;
			y = m_y;
			return POSITION_E;
		}
		if (m_AvailPos & POSITION_W) {
			x = m_x - m_width / 2.0;
			y = m_y;
			return POSITION_W;
		}
		if (m_AvailPos & POSITION_NE) {
			x = m_x + m_width / 2.0;
			y = m_y - m_height / 2.0;
			return POSITION_NE;
		}
		if (m_AvailPos & POSITION_NW) {
			x = m_x - m_width / 2.0;
			y = m_y - m_height / 2.0;
			return POSITION_NW;
		}
		if (m_AvailPos & POSITION_SE) {
			x = m_x + m_width / 2.0;
			y = m_y + m_height / 2.0;
			return POSITION_SE;
		}
		if (m_AvailPos & POSITION_SW) {
			x = m_x - m_width / 2.0;
			y = m_y + m_height / 2.0;
			return POSITION_SW;
		}
	}
	angle = m_AngleList.front ();
	double dir = 0.0, max = 0.0;
	end = m_AngleList.end ();
	//if we are there, there are at least two bonds
	for (n = m_AngleList.begin (), n++; n != end; n++) {
		if (*n - angle > max) {
			if (*n - angle - max > 0.1)
				x = (*n + angle) / 2;
			if (m_nH) {
				if (m_HPos && ((x > 225.0) || (x < 135.0)))
					dir = x;
				else if (m_HPos && (x > 45.0) && (x < 315.0))
					dir = x;
			} else
				dir = x;
			max = *n - angle;
		}
		angle = *n;
	}
	max = sqrt (square (m_width) + square (m_height)) / 2.0 + 24.;//Could do better, should replace 24 by something more intelligent
	x = m_x + max * cos (dir / 180.0 * M_PI);
	y = m_y - max * sin (dir / 180.0 * M_PI);
	return 0;
}

bool Atom::LoadNode (xmlNodePtr)
{
	SetZ (GetZ ());
	return true;
}

void Atom::SetSelected (GtkWidget* w, int state)
{
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	GnomeCanvasGroup* group = pData->Items[this];
	gpointer item;
	gchar const *color, *chargecolor;
	
	switch (state) {	
	case SelStateUnselected:
		color = "white";
		chargecolor = "black";
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
		color = "white";
		chargecolor = "black";
		break;
	}
	g_object_set (G_OBJECT(g_object_get_data(G_OBJECT(group), "rect")),
				"fill_color", color, NULL);
	if ((item = g_object_get_data (G_OBJECT (group), "bullet")))
		g_object_set (item, "fill_color", chargecolor, NULL);
	if ((item = g_object_get_data (G_OBJECT (group), "figure")))
		g_object_set (item, "fill_color", chargecolor, NULL);
	if ((item = g_object_get_data (G_OBJECT (group), "circle")))
		g_object_set (item, "outline_color", chargecolor, NULL);
	if ((item = g_object_get_data (G_OBJECT (group), "sign")))
		g_object_set (item, "outline_color", chargecolor, NULL);
	Object::SetSelected (w, state);
}

bool Atom::AcceptNewBonds (int nb)
{
	if ((m_Valence > 0) || m_ChargeAuto)
		return Element::GetMaxBonds (m_Z) >= (GetTotalBondsNumber () + GetChildrenNumber () + nb);
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	unsigned nel = 0;
	while (electron){ 
		if (electron->IsPair ())
			nel += 2;
		else
			nel++;
		electron = (Electron*) GetNextChild (i);
	}
	nel += GetTotalBondsNumber ();
	return (m_ValenceOrbitals - GetTotalBondsNumber () - GetChildrenNumber () > 0)
			&& (((m_Element->GetValenceElectrons() - m_Charge) > nel) || m_ChargeAuto);
}

void Atom::AddToMolecule (Molecule* Mol)
{
	Mol->AddAtom (this);
}

void Atom::BuildItems (WidgetData* pData)
{
	GnomeCanvasGroup* group = pData->Items[this];
	void* item;
	View* pView = pData->m_View;
	Theme *pTheme = pView->GetDoc ()->GetTheme ();
	double x, y;
	m_width =  m_height = 2.0 * pTheme->GetPadding ();
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	if ((GetZ() != 6) || (GetBondsNumber() == 0) || m_ShowSymbol) {
		int sw, sp;
		const gchar* symbol = GetSymbol (), *text;
		PangoRectangle rect;
		sw = strlen (symbol);
		pango_layout_set_text (m_Layout, symbol, sw);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		m_width += rect.width / PANGO_SCALE;
		int n = GetAttachedHydrogens ();
		PangoAttrList *pal = pango_attr_list_new ();
		if (n > 0) {
			if (n > 1) {
				gchar const *nb =  g_strdup_printf ("%d", n);
				int np, nw = strlen (nb);
				if (m_HPos) {
					text = g_strconcat (symbol, "H", nb, NULL);
					np = sw + 1;
					sp = 0;
				} else {
					text = g_strconcat ("H", nb, symbol, NULL);
					np = 1;
					sp = np + nw;
				}
				pango_layout_set_text (m_Layout, text, -1);
				PangoAttribute *attr = pango_attr_font_desc_new (pView->GetPangoSmallFontDesc());
				attr->start_index = np;
				attr->end_index = np + nw;
				pango_attr_list_insert (pal, attr);
				attr = pango_attr_rise_new (-2 * PANGO_SCALE);
				attr->start_index = np;
				attr->end_index = np + nw;
				pango_attr_list_insert (pal, attr);
			} else {
				if (m_HPos) {
					text = g_strconcat (symbol, "H", NULL);
					sp = 0;
				} else {
					text = g_strconcat ("H", symbol, NULL);
					sp = 1;
				}
				pango_layout_set_text (m_Layout, text, -1);
			}
			pango_layout_set_attributes (m_Layout, pal);
			pango_attr_list_unref (pal);
		} else {
			text = g_strdup (symbol);
			sp = 0;
			pango_layout_set_text (m_Layout, text, -1);
		}
		pango_layout_get_extents (m_Layout, NULL, &rect);
		m_length = double (rect.width / PANGO_SCALE);
		m_text_height = m_height = rect.height / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, sp, &rect);
		int st = rect.x / PANGO_SCALE;
		pango_layout_index_to_pos (m_Layout, sp + sw, &rect);
		m_lbearing = (st + rect.x / PANGO_SCALE) / 2.;

		item = g_object_get_data (G_OBJECT (group), "rect");
		g_object_set (item,
							"x1", x - m_lbearing - pTheme->GetPadding (),
							"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding (),
							"x2", x - m_lbearing + m_length + pTheme->GetPadding (),
							"y2", y  - m_ascent + m_CHeight + m_height + pTheme->GetPadding (),
							NULL);
		
		item = g_object_get_data (G_OBJECT (group), "symbol");
		if (item)
			g_object_set (item,
							"x", x - m_lbearing,
							"y", y - m_ascent + m_CHeight,
							NULL);
		else {
			item = gnome_canvas_item_new (
							group,
							gnome_canvas_pango_get_type(),
							"layout", m_Layout,
							"x", x - m_lbearing,
							"y", y - m_ascent + m_CHeight,
							NULL);
			g_object_set_data (G_OBJECT (group), "symbol", item);
			g_object_set_data (G_OBJECT (item), "object", this);
			g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), pData->Canvas);
			gnome_canvas_item_raise_to_top (GNOME_CANVAS_ITEM (group));
		}
		item = g_object_get_data (G_OBJECT (group), "bullet");
		if (item) {
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data (G_OBJECT (group), "bullet", NULL);
		}
	} else {
		item = g_object_get_data (G_OBJECT (group), "rect");
		g_object_set (item,
							"x1", x - 3,
							"y1", y - 3,
							"x2", x + 3,
							"y2", y + 3,
							NULL);
		item = g_object_get_data (G_OBJECT (group), "symbol");
		if (item) {
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data (G_OBJECT (group), "symbol", NULL);
		}
		item = g_object_get_data (G_OBJECT (group), "bullet");
		if (m_DrawCircle) {
			if (!item) {
				double dx = pTheme->GetStereoBondWidth () / 2.;
				item = gnome_canvas_item_new(
										group,
										gnome_canvas_ellipse_ext_get_type (),
										"x1", x - dx,
										"y1", y - dx,
										"x2", x + dx,
										"y2", y + dx,
										"fill_color",  (pData->IsSelected (this))? SelectColor: Color,
										NULL);
				g_object_set_data (G_OBJECT (group), "bullet", item);
				g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), pData->Canvas);
				g_object_set_data (G_OBJECT (item), "object", this);
			}
		} else if (item) {
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data (G_OBJECT (group), "bullet", NULL);
		}
		m_length = m_text_height = 0;
		gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM (group));
	}
	m_width /= pTheme->GetZoomFactor ();
	m_height /= pTheme->GetZoomFactor ();
	if (m_Changed > 0)
		m_Changed--;
}
	
double Atom::GetYAlign ()
{
	return m_y;
}

bool Atom::HasImplicitElectronPairs ()
{
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	if (m_Valence > 0) {
		int nexplp = 0; //nexplp is the number of explicit lone pairs
		while (electron){ 
			if (electron->IsPair ())
				nexplp++;
			electron = (Electron*) GetNextChild (i);
		}
		return (m_nlp > nexplp);
	}
	unsigned nel = 0;
	while (electron){ 
		if (electron->IsPair ())
			nel += 2;
		else
			nel++;
		electron = (Electron*) GetNextChild (i);
	}
	nel += GetTotalBondsNumber ();
	int nocc = GetChildrenNumber () + GetTotalBondsNumber ();
	return (nocc < m_ValenceOrbitals) && (((m_Element->GetValenceElectrons() - m_Charge) > nel + 1) || m_ChargeAuto);
}

bool Atom::MayHaveImplicitUnpairedElectrons ()
{
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	unsigned nel = 0;
	while (electron){ 
		if (electron->IsPair ())
			nel += 2;
		else
			nel++;
		electron = (Electron*) GetNextChild (i);
	}
	nel += GetTotalBondsNumber ();
	return (m_ValenceOrbitals - GetTotalBondsNumber () - GetChildrenNumber () > 0)
			&& (((m_Element->GetValenceElectrons() - m_Charge) > nel) || m_ChargeAuto);
}

bool Atom::GetPosition(double angle, double& x, double& y)
{
	if (angle > 360.)
		angle -= 360;
	else if (angle < 0.)
		angle += 360;
	if (((GetZ() == 6) && (m_Bonds.size() != 0)) ||
		 !m_nH || ((!m_HPos && (angle < 135. || angle > 225.)) ||
			(m_HPos && (angle > 45. && angle < 315.)))) {
		double t = tan (angle / 180. * M_PI);
		double limit = atan (m_height / m_width) * 180. / M_PI;
		if (angle < limit) {
			x = m_x +  12. + m_width / 2.;
			y = m_y - m_width / 2. * t;
		} else if (angle < 180. - limit) {
			if (!isnan (t))
				x = m_x + m_height / 2. / t;
			else
				x = m_x;
			y = m_y - m_height / 2.;
		} else if (angle < 180. + limit) {
			x = m_x - 12. - m_width / 2.;
			y = m_y + m_width / 2. * t;
		} else if (angle < 360. - limit) {
			if (!isnan (t))
				x = m_x - m_height / 2. / t;
			else
				x = m_x;
			y = m_y + m_height / 2.;
		} else {
			x = m_x +  12. + m_width / 2.;
			y = m_y - m_width / 2. * t;
		}			
		return true;
	}
	return false;
}

void Atom::AddElectron (Electron* electron)
{
	AddChild (electron);
	Update ();
}

void Atom::RemoveElectron (Electron* electron)
{
	// remove the electron from children so that it is not taken into account when
	// updating.
	electron->SetParent (NULL);
	Update ();
	// Force view update.
	Document *pDoc = reinterpret_cast<Document*> (GetDocument ());
	if (pDoc)
		pDoc->GetView ()->Update (this);
}

void Atom::NotifyPositionOccupation (unsigned char pos, bool occupied)
{
	if (occupied)
		m_OccupiedPos |= pos;
	else
		m_OccupiedPos &= ~pos;
}
	
xmlNodePtr Atom::Save (xmlDocPtr xml)
{
	xmlNodePtr node = gcu::Atom::Save (xml), child;
	if (node) {
	// Save electrons
		map<string, Object*>::iterator i;
		Electron* electron = (Electron*) GetFirstChild (i);
		while (electron){ 
			child = electron->Save (xml);
			if (child)
				xmlAddChild (node, child);
			electron = (Electron*) GetNextChild (i);
		}
	}
	if (m_Charge && !m_ChargeAutoPos) {
		char *buf;
		if (m_ChargePos) {
			char const *buf;
			switch (m_ChargePos) {
			case CHARGE_NE:
				buf = "ne";
				break;
			case CHARGE_NW:
				buf = "nw";
				break;
			case CHARGE_N:
				buf = "n";
				break;
			case CHARGE_SE:
				buf = "se";
				break;
			case CHARGE_SW:
				buf = "sw";
				break;
			case CHARGE_S:
				buf = "s";
				break;
			case CHARGE_E:
				buf = "e";
				break;
			case CHARGE_W:
				buf = "w";
				break;
			default:
				buf = "def"; // should not occur
			}
			xmlNewProp (node, (xmlChar*) "charge-position", (xmlChar*) buf);
		} else {
			buf = g_strdup_printf ("%g", m_ChargeAngle * 180. / M_PI);
			xmlNewProp (node, (xmlChar*) "charge-angle", (xmlChar*) buf);
			g_free (buf);
		}
		if (m_ChargeDist != 0.) {
			buf = g_strdup_printf ("%g", m_ChargeDist);
			xmlNewProp (node, (xmlChar*) "charge-dist", (xmlChar*) buf);
			g_free (buf);
		}
	}
	if (GetZ () == 6 && m_ShowSymbol) {
		xmlNewProp (node, (xmlChar*) "show-symbol", (xmlChar*) "true");
	}
	return node;
}
	
bool Atom::Load (xmlNodePtr node)
{
	if (!gcu::Atom::Load (node))
		return false;
	//Load electrons
	xmlNodePtr child = node->children;
	Electron *electron;
	while (child) {
		electron = NULL;
		if (!strcmp ((char*) child->name, "electron"))
			electron = new Electron (this, false);
		else if (!strcmp ((char*) child->name, "electron-pair"))
			electron = new Electron (this, true);
		if (electron && !electron->Load (child))
			return false;
		child = child->next;
	}
	char *buf = (char*) xmlGetProp (node, (xmlChar*) "charge-position");
	m_ChargePos = 0xff;
	if (buf) {
		if (! strcmp (buf, "ne")) {
			m_ChargePos = CHARGE_NE;
			m_ChargeAngle = M_PI / 4.;
		} else if (! strcmp (buf, "nw")) {
			m_ChargePos = CHARGE_NW;
			m_ChargeAngle = 3. * M_PI / 4.;
		} else if (! strcmp (buf, "n")) {
			m_ChargePos = CHARGE_N;
			m_ChargeAngle = M_PI / 2.;
		} else if (! strcmp (buf, "se")) {
			m_ChargePos = CHARGE_SE;
			m_ChargeAngle = 7. * M_PI / 4;
		} else if (! strcmp (buf, "sw")) {
			m_ChargePos = CHARGE_SW;
			m_ChargeAngle = 5. * M_PI / 4;
		} else if (! strcmp (buf, "s")) {
			m_ChargePos = CHARGE_S;
			m_ChargeAngle = 3 * M_PI / 2.;
		} else if (! strcmp (buf, "e")) {
			m_ChargePos = CHARGE_E;
			m_ChargeAngle = 0.;
		} else if (! strcmp (buf, "w")) {
			m_ChargePos = CHARGE_W;
			m_ChargeAngle = M_PI;
		}
		m_ChargeAutoPos = false;
		xmlFree (buf);
	} else {
		buf = (char*) xmlGetProp(node, (xmlChar*)"charge-angle");
		if (buf) {
			sscanf(buf, "%lg", &m_ChargeAngle);
			m_ChargeAngle *= M_PI / 180.;
			xmlFree (buf);
			m_ChargePos = 0;
			m_ChargeAutoPos = false;
		}
	}
	buf = (char*) xmlGetProp(node, (xmlChar*)"charge-dist");
	if (buf) {
		sscanf(buf, "%lg", &m_ChargeDist);
		xmlFree (buf);
		m_ChargeAutoPos = false;
	} else
		m_ChargeDist = 0.;
	buf = (char*) xmlGetProp (node, (xmlChar*) "show-symbol");
	if (buf) {
		if (!strcmp (buf, "true"))
			m_ShowSymbol = true;
		xmlFree (buf);
	}
	return true;
}
	
bool Atom::AcceptCharge (int charge) {
	unsigned nb = GetTotalBondsNumber (), ne = 0;
	map<string, Object*>::iterator i;
	Electron* electron = (Electron*) GetFirstChild (i);
	while (electron){ 
		if (electron->IsPair ())
			ne += 2;
		else
			ne++;
		electron = (Electron*) GetNextChild (i);
	}
	if (charge < 0)
		return (m_Element->GetTotalValenceElectrons () <= m_Element->GetMaxValenceElectrons () + charge - nb - 2 * GetChildrenNumber () + ne);
	if (nb)
		return (m_Element->GetValenceElectrons () >= charge + nb + ne);
	return (charge <= GetZ ());
}

void Atom::SetChargePosition (unsigned char Pos, bool def, double angle, double distance)
{
	if (Pos != m_ChargePos) {
		m_ChargeAutoPos = def;
		if (m_ChargePos > 0)
			NotifyPositionOccupation (m_ChargePos, false);
		m_ChargePos = Pos;
		if (m_ChargePos > 0)
			NotifyPositionOccupation (m_ChargePos, true);
	}
	m_ChargeAngle = angle;
	m_ChargeDist = distance;
	m_AvailPosCached = false;
}

char Atom::GetChargePosition (double *Angle, double *Dist)
{
	if (Angle)
		*Angle = m_ChargeAngle;
	if (Dist)
		*Dist = m_ChargeDist;
	return (m_ChargeAutoPos)? -1: m_ChargePos;
}

void Atom::SetCharge (int charge)
{
	gcu::Atom::SetCharge (charge);
	m_ChargeAuto = false;
	Update ();
}

void Atom::Transform2D (Matrix2D& m, double x, double y)
{
	Atom::Transform2D (m, x, y);
	// Now transform electrons
	map<string, Object*>::iterator i;
	Object* electron = GetFirstChild (i);
	while (electron) {
		electron->Transform2D (m, x, y);
		electron = GetNextChild (i);
	}
	if (GetCharge ()) {
		if (m_ChargeAutoPos) {
			if (m_ChargePos > 0)
				NotifyPositionOccupation (m_ChargePos, false);
			m_ChargePos = 0xff;
			Update ();
		} else {
			double xc = cos (m_ChargeAngle), yc = - sin (m_ChargeAngle);
			m.Transform (xc, yc);
			m_ChargeAngle = atan2 (- yc, xc);
			if (m_ChargeAngle < 0)
				m_ChargeAngle += 2 * M_PI;
			SetChargePosition (0, FALSE, m_ChargeAngle, m_ChargeDist);
		}
	}
}

static void do_display_symbol (GtkToggleAction *action, Atom *pAtom)
{
	Document *Doc = static_cast <Document *> (pAtom->GetDocument ());
	Operation *Op = Doc->GetNewOperation (GCP_MODIFY_OPERATION);
	Object *Obj = pAtom->GetGroup ();
	Op->AddObject (Obj, 0);
	pAtom->SetShowSymbol (gtk_toggle_action_get_active (action));
	pAtom->Update ();
	pAtom->ForceChanged ();
	pAtom->EmitSignal (OnChangedSignal);
	Op->AddObject (Obj, 1);
	Doc->FinishOperation ();
	Doc->GetView ()->Update (pAtom);
}

bool Atom::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	bool result = false;
	if (GetZ () == 6 && GetBondsNumber() != 0) {
		GtkActionGroup *group = gtk_action_group_new ("atom");
		GtkAction *action;
		action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
		gtk_action_group_add_action (group, action);
		action = GTK_ACTION (gtk_toggle_action_new ("show-symbol", _("Display symbol"),  _("Whether to display carbon atom symbol or not"), NULL));
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), m_ShowSymbol);
		g_signal_connect (action, "toggled", G_CALLBACK (do_display_symbol), this);
		gtk_action_group_add_action (group, action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='show-symbol'/></menu></popup></ui>", -1, NULL);
		gtk_ui_manager_insert_action_group (UIManager, group, 0);
		result = true;
	}
	return result | GetParent ()->BuildContextualMenu (UIManager, object, x, y);
}

}	//	namespace gcp
