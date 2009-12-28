// -*- C++ -*-

/* 
 * GChemPaint library
 * atom.cc
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bond.h"
#include "document.h"
#include "electron.h"
#include "molecule.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include "Hposdlg.h"
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/text.h>
#include <gccv/text-tag.h>
#include <gcu/element.h>
#include <openbabel/mol.h>
#include <glib/gi18n-lib.h>
#include <cstdlib>
#include <cstring>

using namespace gcu;
using namespace OpenBabel;
using namespace std;

#define ATOM_EPSILON 0.1

namespace gcp {

Atom::Atom ():
	gcu::Atom (),
	ItemClient (),
	m_ShowSymbol (false),
	m_HPosStyle (AUTO_HPOS)
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
	m_Layout = m_ChargeLayout = m_HLayout = NULL;
	m_DrawCircle = false;
	m_SWidth = 0.;
	m_ChargeItem = NULL;
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

Atom::Atom (int Z, double x, double y, double z):
	gcu::Atom (Z, x, y, z),
	ItemClient (),
	m_ShowSymbol (false),
	m_HPosStyle (AUTO_HPOS)
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
	m_Layout = m_ChargeLayout = m_HLayout = NULL;
	m_DrawCircle = false;
	m_SWidth = 0.;
	m_ChargeItem = NULL;
}

Atom::Atom (OBAtom* atom):
	gcu::Atom (),
	ItemClient (),
	m_ShowSymbol (false),
	m_HPosStyle (AUTO_HPOS)
{
	m_x = atom->GetX ();
	m_y = - atom->GetY ();
	m_z = atom->GetZ ();
	m_nlp = 0;
	SetZ (atom->GetAtomicNum ());
	gchar* Id = g_strdup_printf ("a%d", atom->GetIdx());
	SetId (Id);
	g_free (Id);
	m_HPos = GetBestSide ();
	m_ascent = 0;
	m_CHeight = 0.;
	m_Changed = 0;
	m_AvailPosCached = false;
	m_OccupiedPos = 0;
	m_ChargePos = 0xff;
	m_ChargeAngle = 0.;
	m_ChargeDist = 0.;
	m_ChargeAutoPos = true;
	m_Layout = m_ChargeLayout = m_HLayout = NULL;
	m_DrawCircle = false;
	m_Charge = atom->GetFormalCharge ();
	m_SWidth = 0.;
	m_ChargeItem = NULL;
}

void Atom::SetZ (int Z)
{
	gcu::Atom::SetZ (Z);
	if (Z < 1)
		return;
	m_Element = Element::GetElement (m_Z);
	if ((m_Valence = (m_Element)? m_Element->GetDefaultValence (): 0))
		m_HPos = (m_HPosStyle == AUTO_HPOS)? GetBestSide(): m_HPosStyle;
	else
		m_nH = 0;
	int max = (m_Element)? m_Element->GetMaxValenceElectrons (): 0;
	int diff = (m_Element)? m_Element->GetTotalValenceElectrons () - m_Element->GetValenceElectrons (): 0;
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

int Atom::GetTotalBondsNumber () const
{
	std::map<gcu::Atom*, gcu::Bond*>::const_iterator i, end = m_Bonds.end ();
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

HPos Atom::GetBestSide ()
{
	size_t nb_bonds = m_Bonds.size ();
	if (nb_bonds == 0)
		return (Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS;
	std::map<gcu::Atom*, gcu::Bond*>::iterator i, end = m_Bonds.end();
	double sumc = 0.0, sums = 0.0, a;
	for (i = m_Bonds.begin(); i != end; i++) {
		a = ((Bond*) (*i).second)->GetAngle2DRad (this);
		sumc += cos (a);
		sums += sin (a);
	}
	if (fabs (sums) > fabs (sumc) && nb_bonds > 1)
		return (fabs (sums) > .1)? ((sums >= 0.)? BOTTOM_HPOS: TOP_HPOS): ((Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS);
	else
		return (fabs (sumc) > .1)? ((sumc >= 0.)? LEFT_HPOS: RIGHT_HPOS): ((Element::BestSide (m_Z))? RIGHT_HPOS: LEFT_HPOS);
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
	if (m_Valence > 0 && !m_Element->IsMetallic ()) {
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
		m_HPos = (m_HPosStyle == AUTO_HPOS)? GetBestSide(): m_HPosStyle;
	} else {
		m_nH = 0;
		if (m_ChargeAuto || !m_Charge) {
			m_Charge = (m_Element)? m_Element->GetValenceElectrons () - 2 * nexplp - nexplu - nbonds: 0;
			if (m_Charge > 0)
				m_Charge = 0;
			m_ChargeAuto = true;
		}
	}
	Document *pDoc = (Document *) GetDocument ();
	if (pDoc && pDoc->GetView ())
		m_Changed = pDoc->GetView ()->GetNbWidgets ();
	m_AvailPosCached = false;
	map<gcu::Atom*, gcu::Bond*>::iterator j = m_Bonds.begin(), jend = m_Bonds.end ();
	if (nbonds && GetZ () == 6) {
		// update large bonds ends
		Bond *bond;
		BondType type;
		bool DrawCircle;
		int nb = 0;
		j = m_Bonds.begin();
		while (j != jend)
		{
			bond = dynamic_cast<Bond*> ((Bond*)(*j).second);
			type = bond->GetType ();
			if (type == ForeBondType || (type == UpBondType && bond->GetAtom (1) == this))
				nb++;
			j++;
		}
		DrawCircle = nb > 1;
		if (!DrawCircle && GetBondsNumber () == 2) {
			j = m_Bonds.begin();
			double angle = static_cast<Bond*> ((*j).second)->GetAngle2D (this);
			j++;
			angle -= static_cast<Bond*> ((*j).second)->GetAngle2D (this);
			if (go_finite (angle)) {
				while (angle < 0)
					angle += 360.;
				while (angle > 360.)
					angle -= 360;
				if (fabs (angle - 180.) < 1)
					DrawCircle = true;
			}
		}
		if (DrawCircle != m_DrawCircle) {
			m_DrawCircle = DrawCircle;
			m_Changed = true;
		}
	}
	// Update all double bonds
	for (j = m_Bonds.begin(); j != jend; j++)
		if (((*j).second)->GetOrder () == 2)
			static_cast<Bond*> ((*j).second)->SetDirty ();
}

/*void Atom::Add (GtkWidget* w) const
{
	if (!w || !GetZ ())
		return;
	if (m_Changed > 0)
		const_cast <Atom *> (this)->m_Changed--;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] != NULL)
		return;
	View* pView = pData->m_View;
	Theme *pTheme = pView->GetDoc ()->GetTheme ();
	if (m_Layout == NULL) {
		PangoContext *pc = gccv::Text::GetContext ();
		const_cast <Atom *> (this)->m_Layout = pango_layout_new (pc);
		const_cast <Atom *> (this)->m_HLayout = pango_layout_new (pc);
	}
	if (m_FontName != pView->GetFontName ()) {
		pango_layout_set_font_description (m_Layout, pView->GetPangoFontDesc ());
		pango_layout_set_text (m_Layout, "l", 1);
		PangoLayoutIter* iter = pango_layout_get_iter (m_Layout);
		const_cast <Atom *> (this)->m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		const_cast <Atom *> (this)->m_FontName = pView->GetFontName ();
		const_cast <Atom *> (this)->m_CHeight = 0.;
	}
	PangoRectangle rect;
	if (m_CHeight == 0.) {
		pango_layout_set_text (m_Layout, "C", 1);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		const_cast <Atom *> (this)->m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
	}
	double x, y, xc = 0., yc;
	const_cast <Atom *> (this)->m_width = const_cast <Atom *> (this)->m_height = 2.0 * pTheme->GetPadding ();
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	GnomeCanvasItem *item;
	GnomeCanvasGroup *group, *chgp;
	group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	g_signal_connect (G_OBJECT (group), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "object", (void *) this);
	if ((GetZ () != 6) || (GetBondsNumber () == 0)) {
		int sw;
		const gchar* symbol = GetSymbol (), *text;
		sw = strlen (symbol);
		pango_layout_set_text (m_Layout, symbol, sw);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		const_cast <Atom *> (this)->m_width += rect.width / PANGO_SCALE;
		const_cast <Atom *> (this)->BuildSymbolGeometry ((double) rect.width / PANGO_SCALE, (double) rect.height / PANGO_SCALE, m_ascent - (double) rect.y / PANGO_SCALE - m_CHeight);
		const_cast <Atom *> (this)->m_lbearing = m_width / 2.;
		const_cast <Atom *> (this)->m_xROffs = const_cast <Atom *> (this)->m_yROffs = 0.;
		pango_layout_get_extents (m_Layout, NULL, &rect);
		const_cast <Atom *> (this)->m_length = double (rect.width / PANGO_SCALE);
		const_cast <Atom *> (this)->m_text_height = const_cast <Atom *> (this)->m_height = rect.height / PANGO_SCALE;
		int n = GetAttachedHydrogens ();
		if (n > 0) {
			pango_layout_set_text (m_HLayout, "H", -1);
			PangoRectangle HRect;
			HRect.width = 0.;
			pango_layout_get_extents (m_HLayout, &HRect, NULL);
			switch (m_HPos) {
			case LEFT_HPOS:
				//the x offset needs to be calculated after adding possible stoichimoetry.
				const_cast <Atom *> (this)->m_yHOffs = 0.;
				break;
			case RIGHT_HPOS:
				const_cast <Atom *> (this)->m_xHOffs = rect.width / PANGO_SCALE + 1.;
				const_cast <Atom *> (this)->m_yHOffs = 0.;
				break;
			case TOP_HPOS:
				const_cast <Atom *> (this)->m_xHOffs = m_lbearing - pTheme->GetPadding () - HRect.width / PANGO_SCALE / 2.;
				break;
			case BOTTOM_HPOS:
				const_cast <Atom *> (this)->m_xHOffs = m_lbearing - pTheme->GetPadding () - HRect.width / PANGO_SCALE / 2.;
				const_cast <Atom *> (this)->m_yHOffs = m_CHeight * 2. + pTheme->GetPadding ();
				break;
			default:
				g_critical ("This should not happen, please file a bug report");
				break;
			}
			if (n > 1) {
				gchar const *nb =  g_strdup_printf ("%d", n);
				int nw = strlen (nb) + 1;
				PangoAttrList *pal = pango_attr_list_new ();
				text = g_strconcat ("H", nb, NULL);
				pango_layout_set_text (m_HLayout, text, -1);
				nw = strlen (text);
				PangoAttribute *attr = pango_attr_font_desc_new (pView->GetPangoSmallFontDesc());
				attr->start_index = 1;
				attr->end_index = nw;
				pango_attr_list_insert (pal, attr);
				attr = pango_attr_rise_new (-2 * PANGO_SCALE);
				attr->start_index = 1;
				attr->end_index = nw;
				pango_attr_list_insert (pal, attr);
				pango_layout_set_attributes (m_HLayout, pal);
				pango_attr_list_unref (pal);
			}
			pango_layout_get_extents (m_HLayout, &HRect, NULL);
			// evaluate underlying rectangle size and position
			if (HRect.width > 0) {
				switch (m_HPos) {
				case LEFT_HPOS:
					const_cast <Atom *> (this)->m_xHOffs = -HRect.width / PANGO_SCALE - 1.;
					const_cast <Atom *> (this)->m_xROffs = m_xHOffs;
					const_cast <Atom *> (this)->m_length += HRect.width / PANGO_SCALE + 1.;
					break;
				case RIGHT_HPOS:
					const_cast <Atom *> (this)->m_length += HRect.width / PANGO_SCALE + 1.;
					break;
				case TOP_HPOS:
					const_cast <Atom *> (this)->m_yHOffs = -HRect.height / PANGO_SCALE - pTheme->GetPadding ();
					const_cast <Atom *> (this)->m_yROffs = -HRect.height / PANGO_SCALE - pTheme->GetPadding ();
					const_cast <Atom *> (this)->m_text_height += HRect.height / PANGO_SCALE + pTheme->GetPadding ();
					const_cast <Atom *> (this)->m_length = MAX (m_length, HRect.width / PANGO_SCALE);
					break;
				case BOTTOM_HPOS:
					const_cast <Atom *> (this)->m_text_height += HRect.height / PANGO_SCALE + pTheme->GetPadding ();
					const_cast <Atom *> (this)->m_length = MAX (m_length, HRect.width / PANGO_SCALE);
					break;
				default:
					break;
				}
			}
		}
		item = gnome_canvas_item_new (
							group,
							gnome_canvas_rect_ext_get_type (),
							"x1", x - m_lbearing + m_xROffs,
							"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding () + m_yROffs,
							"x2", x - m_lbearing + m_length + 2 * pTheme->GetPadding () + m_xROffs,
							"y2", y  - m_ascent + m_CHeight + m_text_height + pTheme->GetPadding () + m_yROffs,
							"fill_color", (pData->IsSelected (this))? SelectColor: NULL,
							NULL);
		g_object_set_data (G_OBJECT (group), "rect", item);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		gnome_canvas_item_hide (item);
		
		item = gnome_canvas_item_new (
							group,
							gnome_canvas_pango_get_type (),
							"x", x - m_lbearing,
							"y", y - m_ascent + m_CHeight,
							"layout", m_Layout,
						NULL);
		g_object_set_data (G_OBJECT (group), "symbol", item);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		item = gnome_canvas_item_new (
							group,
							gnome_canvas_pango_get_type (),
							"x", x - m_lbearing + m_xHOffs,
							"y", y - m_ascent + m_CHeight + m_yHOffs,
							"layout", m_HLayout,
						NULL);
		g_object_set_data (G_OBJECT (group), "hydrogens", item);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	} else {
		item = gnome_canvas_item_new (
								group,
								gnome_canvas_rect_ext_get_type (),
								"x1", x - 3,
								"y1", y - 3,
								"x2", x + 3,
								"y2", y + 3,
								"fill_color",  (pData->IsSelected (this))? SelectColor: NULL,
								NULL);
		g_object_set_data(G_OBJECT (group), "rect", item);
		gnome_canvas_request_redraw ((GnomeCanvas*) w, (int) x - 3, (int) y - 3, (int) x + 3, (int) y + 3);
		gnome_canvas_item_lower_to_bottom (GNOME_CANVAS_ITEM (group));
		gnome_canvas_item_raise (GNOME_CANVAS_ITEM (group), 1);
		g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
		g_object_set_data (G_OBJECT (item), "object", (void *) this);
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
			g_object_set_data (G_OBJECT (item), "object", (void *) this);
		}
	}
	pData->Items[this] = group;
	const_cast <Atom *> (this)->m_width /= pTheme->GetZoomFactor ();
	const_cast <Atom *> (this)->m_height /= pTheme->GetZoomFactor ();*/
	/* add charge */
/*	int charge = GetCharge ();
	if (charge) {
		int align = const_cast <Atom *> (this)->GetChargePosition (const_cast <Atom *> (this)->m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
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
				PangoContext* pc = gccv::Text::GetContext();
				const_cast <Atom *> (this)->m_ChargeLayout = pango_layout_new (pc);
				pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
			}
			pango_layout_set_text (m_ChargeLayout, fig, -1);
			pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
			const_cast <Atom *> (this)->m_ChargeWidth = rect.width / PANGO_SCALE;
			const_cast <Atom *> (this)->m_ChargeTWidth = m_ChargeWidth + 1. + pTheme->GetChargeSignSize ();
		} else {
			const_cast <Atom *> (this)->m_ChargeWidth = 0.;
			const_cast <Atom *> (this)->m_ChargeTWidth = pTheme->GetChargeSignSize ();
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
	map<string, Object*>::const_iterator i;
	Object const *electron = GetFirstChild (i);
	while (electron){
		electron->Add (w);
		electron = GetNextChild (i);
	}
}*/

/*void Atom::Update (GtkWidget* w) const
{
	if (!w || !GetZ ())
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] == NULL)
		return;
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y, xc = 0., yc;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	GnomeCanvasGroup *group = pData->Items[this];
	if (m_FontName != pData->m_View->GetFontName ()) {
		View *pView = pData->m_View;
		PangoContext* pc = gccv::Text::GetContext ();
		PangoLayout *Layout = pango_layout_new (pc);
		pango_layout_set_font_description (Layout, pView->GetPangoFontDesc ());
		pango_layout_set_font_description (m_Layout, pView->GetPangoFontDesc ());
		pango_layout_set_text (Layout, "l", 1);
		PangoLayoutIter* iter = pango_layout_get_iter (Layout);
		const_cast <Atom *> (this)->m_ascent = pango_layout_iter_get_baseline (iter) / PANGO_SCALE;
		pango_layout_iter_free (iter);
		const_cast <Atom *> (this)->m_FontName = pView->GetFontName ();
		pango_layout_set_text (Layout, "C", 1);
		PangoRectangle rect;
		pango_layout_get_extents (Layout, &rect, NULL);
		const_cast <Atom *> (this)->m_CHeight =  double (rect.height / PANGO_SCALE) / 2.0;
		g_object_unref (G_OBJECT (Layout));
	}
	if (m_Changed)
		const_cast <Atom *> (this)->BuildItems (pData);
	else {
		if ((GetZ() != 6) || (GetBondsNumber () == 0) || m_ShowSymbol) {
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "symbol")),
								"x", x - m_lbearing,
								"y", y - m_ascent + m_CHeight,
								NULL);
			if (GetAttachedHydrogens ())
				g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "hydrogens")),
									"x", x - m_lbearing + m_xHOffs,
									"y", y - m_ascent + m_CHeight + m_yHOffs,
									NULL);
			g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "rect")),
								"x1", x - m_lbearing + m_xROffs,
								"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding () + m_yROffs,
								"x2", x - m_lbearing + m_length + 2 * pTheme->GetPadding () + m_xROffs,
								"y2", y  - m_ascent + m_CHeight + m_text_height + pTheme->GetPadding () + m_yROffs,
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
			int align = const_cast <Atom *> (this)->GetChargePosition (const_cast <Atom *> (this)->m_ChargePos, m_ChargeAngle * 180. / M_PI, x, y);
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
					PangoContext* pc = gccv::Text::GetContext ();
					const_cast <Atom *> (this)->m_ChargeLayout = pango_layout_new (pc);
					pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
				}
				pango_layout_set_text (m_ChargeLayout, fig, -1);
				pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
				const_cast <Atom *> (this)->m_ChargeWidth = rect.width / PANGO_SCALE;
			} else
				const_cast <Atom *> (this)->m_ChargeWidth = 0.;
			const_cast <Atom *> (this)->m_ChargeTWidth = m_ChargeWidth + 1. + pTheme->GetChargeSignSize ();
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
			int align = const_cast <Atom *> (this)->GetChargePosition(const_cast <Atom *> (this)->m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
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
					PangoContext* pc = gccv::Text::GetContext ();
					const_cast <Atom *> (this)->m_ChargeLayout = pango_layout_new (pc);
					pango_layout_set_font_description (m_ChargeLayout, pData->m_View->GetPangoSmallFontDesc ());
				}
				pango_layout_set_text (m_ChargeLayout, fig, -1);
				PangoRectangle rect;
				pango_layout_get_extents (m_ChargeLayout, NULL, &rect);
				const_cast <Atom *> (this)->m_ChargeWidth = rect.width / PANGO_SCALE;
				const_cast <Atom *> (this)->m_ChargeTWidth = m_ChargeWidth + pTheme->GetPadding () + pTheme->GetChargeSignSize ();
			} else {
				const_cast <Atom *> (this)->m_ChargeWidth = 0.;
				const_cast <Atom *> (this)->m_ChargeTWidth = pTheme->GetChargeSignSize ();
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
	map<string, Object*>::const_iterator i;
	Object const *electron = GetFirstChild (i);
	while (electron){
		electron->Update (w);
		electron = GetNextChild (i);
	}
}*/

void Atom::UpdateAvailablePositions ()
{
	list<double>::iterator n;
	double angle, delta, dir;
	m_AngleList.clear ();
	if (((GetZ() != 6 || m_Bonds.size() == 0)) && m_nH) {
		switch (m_HPos) {
		case LEFT_HPOS:
			m_AvailPos = 0x6D;
			m_AngleList.push_front(225.0);
			m_AngleList.push_front(135.0);
			break;
		case RIGHT_HPOS:
			m_AvailPos = 0xB6;
			m_AngleList.push_front(315.0);
			m_AngleList.push_front(45.0);
			break;
		case TOP_HPOS:
			m_AvailPos = 0xF8;
			m_AngleList.push_front(135.0);
			m_AngleList.push_front(45.0);
			break;
			break;
		case BOTTOM_HPOS:
			m_AvailPos = 0xC7;
			m_AngleList.push_front(315.0);
			m_AngleList.push_front(225.0);
			break;
			break;
		default:
			break;
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
		if ((m_AvailPos & POSITION_SW) && (angle >= 180.0 - ATOM_EPSILON) &&
			(angle <= 270.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_SW;
		if ((m_AvailPos & POSITION_SE) && (((angle >= 270.0 - ATOM_EPSILON) &&
			(angle <= 360.0 + ATOM_EPSILON)) || (fabs(angle) < ATOM_EPSILON)))
			m_AvailPos -= POSITION_SE;
		if ((m_AvailPos & POSITION_S) && (angle >= 225.0 - ATOM_EPSILON) &&
			(angle <= 315.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_S;
		if ((m_AvailPos & POSITION_NW) && (angle >= 90.0 - ATOM_EPSILON) &&
			(angle <= 180.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_NW;
		if ((m_AvailPos & POSITION_NE) && (((angle >= 0.0 - ATOM_EPSILON) &&
			(angle <= 90.0 + ATOM_EPSILON)) || (fabs(angle - 360.0) < ATOM_EPSILON)))
			m_AvailPos -= POSITION_NE;
		if ((m_AvailPos & POSITION_N) && (angle >= 45.0 - ATOM_EPSILON) &&
			(angle <= 135.0 + ATOM_EPSILON))
			m_AvailPos -= POSITION_N;
		if ((m_AvailPos & POSITION_W) && ((angle <= 225.0 + ATOM_EPSILON) &&
			(angle >= 135.0 - ATOM_EPSILON)))
			m_AvailPos -= POSITION_W;
		if ((m_AvailPos & POSITION_E) && ((angle >= 315.0 - ATOM_EPSILON) ||
			(angle <= 45.0 + ATOM_EPSILON)))
			m_AvailPos -= POSITION_E;
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

gccv::Anchor Atom::GetChargePosition (unsigned char& Pos, double Angle, double& x, double& y)
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
			if (m_AvailPos & POSITION_NE)
				Pos = POSITION_NE;
			else if (m_AvailPos & POSITION_NW)
				Pos = POSITION_NW;
			else if (m_AvailPos & POSITION_N)
				Pos = POSITION_N;
			else if (m_AvailPos & POSITION_SE)
				Pos = POSITION_SE;
			else if (m_AvailPos & POSITION_SW)
				Pos = POSITION_SW;
			else if (m_AvailPos & POSITION_S)
				Pos = POSITION_S;
			else if (m_AvailPos & POSITION_E)
				Pos = POSITION_E;
			else if (m_AvailPos & POSITION_W)
				Pos = POSITION_W;
		} else {
			Pos = 0;
			angle = m_AngleList.front();
			double max = 0.0;
			end = m_AngleList.end ();
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
			return gccv::AnchorCenter;
	} else {
		if (Angle > 360.)
			Angle -= 360;
		else if (Angle < 0.)
			Angle += 360;
		if (!(((GetZ() == 6) && (m_Bonds.size() != 0)) ||
			 !m_nH || ((!m_HPos && (Angle < 135. || Angle > 225.)) ||
				(m_HPos && (Angle > 45. && Angle < 315.)))))
			return gccv::AnchorCenter;
	}
	switch (Pos) {
	case POSITION_NE:
		x = m_x + m_width / 2.0;
		y = m_y - m_height / 2.0;
		return gccv::AnchorWest;
	case POSITION_NW:
		x = m_x - m_width / 2.0;
		y = m_y - m_height / 2.0;
		return gccv::AnchorEast;
	case POSITION_N:
		x = m_x;
		y = m_y - m_height / 2.0;
		return gccv::AnchorSouth;
	case POSITION_SE:
		x = m_x + m_width / 2.0;
		y = m_y + m_height / 2.0;
		return gccv::AnchorWest;
	case POSITION_SW:
		x = m_x - m_width / 2.0;
		y = m_y + m_height / 2.0;
		return gccv::AnchorEast;
	case POSITION_S:
		x = m_x;
		y = m_y + m_height / 2.0;
		return gccv::AnchorNorth;
	case POSITION_E:
		x = m_x + m_width / 2.0;
		y = m_y;
		return gccv::AnchorWest;
	case POSITION_W:
		x = m_x - m_width / 2.0;
		y = m_y;
		return gccv::AnchorEast;
	default: {
			double t = tan (Angle / 180. * M_PI);
			double limit = atan (m_height / m_width) * 180. / M_PI;
			if (Angle < limit) {
				x = m_x /*+  12. */+ m_width / 2.;
				y = m_y - m_width / 2. * t;
				return gccv::AnchorWest;
			} else if (Angle < 180. - limit) {
				if (!isnan (t))
					x = m_x + m_height / 2. / t;
				else
					x = m_x;
				y = m_y - m_height / 2.;
				return gccv::AnchorSouth;
			} else if (Angle < 180. + limit) {
				x = m_x /*- 12.*/ - m_width / 2.;
				y = m_y + m_width / 2. * t;
				return gccv::AnchorEast;
			} else if (Angle < 360. - limit) {
				if (!isnan (t))
					x = m_x - m_height / 2. / t;
				else
					x = m_x;
				y = m_y + m_height / 2.;
				return gccv::AnchorNorth;
			} else {
				x = m_x /*+  12.*/ + m_width / 2.;
				y = m_y - m_width / 2. * t;
				return gccv::AnchorWest;
			}
		}			
	}
	return gccv::AnchorCenter; // should not occur
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

void Atom::SetSelected (int state)
{
	GOColor textcolor, othercolor;
	
	switch (state) {	
	default:
	case SelStateUnselected:
		textcolor = 0;
		othercolor = GO_COLOR_BLACK;
		break;
	case SelStateSelected:
		othercolor = textcolor = SelectColor;
		break;
	case SelStateUpdating:
		othercolor = textcolor = AddColor;
		break;
	case SelStateErasing:
		othercolor = textcolor = DeleteColor;
		break;
	}
	gccv::Group *group = static_cast <gccv::Group *> (m_Item);
	std::list<gccv::Item *>::iterator it;
	gccv::Item *item = group->GetFirstChild (it);
	while (item) {
		gccv::FillItem *fill;
		gccv::Text *text;
		if ((text = dynamic_cast <gccv::Text *> (item)))
			text->SetColor (othercolor);
		else if ((fill = dynamic_cast <gccv::Rectangle *> (item)))
			fill->SetFillColor (textcolor);
		else if ((fill = dynamic_cast <gccv::FillItem *> (item)))
			fill->SetFillColor (othercolor);
		else
			static_cast <gccv::LineItem *> (item)->SetLineColor (othercolor);
		item = group->GetNextChild (it);
	}
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

/*void Atom::BuildItems (WidgetData* pData)
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
	m_xROffs = m_yROffs = 0.;
	if ((GetZ() != 6) || (GetBondsNumber() == 0) || m_ShowSymbol) {
		int sw;
		const gchar* symbol = GetSymbol (), *text;
		PangoRectangle rect, HRect;
		sw = strlen (symbol);
		pango_layout_set_text (m_Layout, symbol, sw);
		pango_layout_get_extents (m_Layout, &rect, NULL);
		m_width += rect.width / PANGO_SCALE;
		pango_layout_get_extents (m_Layout, NULL, &rect);
		m_length = double (rect.width / PANGO_SCALE);
		m_text_height = m_height = rect.height / PANGO_SCALE;
		m_lbearing = m_width / 2.;
		HRect.width = 0;
		int n = GetAttachedHydrogens ();
		if (n > 0) {
			pango_layout_set_text (m_HLayout, "H", -1);
			pango_layout_get_extents (m_HLayout, &HRect, NULL);
			switch (m_HPos) {
			case LEFT_HPOS:
				//the x offset needs to be calculated after adding possible stoichimoetry.
				m_xHOffs = m_yHOffs = 0.;
				break;
			case RIGHT_HPOS:
				m_xHOffs = rect.width / PANGO_SCALE + 1.;
				m_yHOffs = 0.;
				break;
			case TOP_HPOS:
				m_xHOffs = m_lbearing - pTheme->GetPadding () - HRect.width / PANGO_SCALE / 2.;
				break;
			case BOTTOM_HPOS:
				m_xHOffs = m_lbearing - pTheme->GetPadding () - HRect.width / PANGO_SCALE / 2.;
				m_yHOffs = m_CHeight * 2. + pTheme->GetPadding ();
				break;
			default:
				g_critical ("This should not happen, please file a bug report");
				break;
			}
			if (n > 1) {
				gchar const *nb =  g_strdup_printf ("%d", n);
				int nw = strlen (nb);
				PangoAttrList *pal = pango_attr_list_new ();
				text = g_strconcat ("H", nb, NULL);
				pango_layout_set_text (m_HLayout, text, -1);
				nw = strlen (text);
				PangoAttribute *attr = pango_attr_font_desc_new (pView->GetPangoSmallFontDesc());
				attr->start_index = 1;
				attr->end_index = strlen (text);
				pango_attr_list_insert (pal, attr);
				attr = pango_attr_rise_new (-2 * PANGO_SCALE);
				attr->start_index = 1;
				attr->end_index = nw;
				pango_attr_list_insert (pal, attr);
				pango_layout_set_attributes (m_HLayout, pal);
				pango_attr_list_unref (pal);
			}
			pango_layout_get_extents (m_HLayout, &HRect, NULL);
			// evaluate underlying rectangle size and position
			if (HRect.width > 0) {
				switch (m_HPos) {
				case LEFT_HPOS:
					m_xHOffs = -HRect.width / PANGO_SCALE - 1.;
					m_xROffs = m_xHOffs;
					m_length += HRect.width / PANGO_SCALE + 1.;
					break;
				case RIGHT_HPOS:
					m_length += HRect.width / PANGO_SCALE + 1.;
					break;
				case TOP_HPOS:
					m_yHOffs = -HRect.height / PANGO_SCALE - pTheme->GetPadding ();
					m_yROffs = -HRect.height / PANGO_SCALE - pTheme->GetPadding ();
					m_text_height += HRect.height / PANGO_SCALE + pTheme->GetPadding ();
					m_length = MAX (m_length, HRect.width / PANGO_SCALE);
					break;
				case BOTTOM_HPOS:
					m_text_height += HRect.height / PANGO_SCALE + pTheme->GetPadding ();
					m_length = MAX (m_length, HRect.width / PANGO_SCALE);
					break;
				default:
					break;
				}
			}
		}
		item = g_object_get_data (G_OBJECT (group), "rect");
		g_object_set (item,
							"x1", x - m_lbearing + m_xROffs,
							"y1", y  - m_ascent + m_CHeight - pTheme->GetPadding () + m_yROffs,
							"x2", x - m_lbearing + m_length + 2 * pTheme->GetPadding () + m_xROffs,
							"y2", y  - m_ascent + m_CHeight + m_text_height + pTheme->GetPadding () + m_yROffs,
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
		item = g_object_get_data (G_OBJECT (group), "hydrogens");
		if (item) {
			if (n == 0) {
				gtk_object_destroy (GTK_OBJECT (item));
				g_object_set_data ((GObject*)group, "hydrogens", NULL);
			} else
				g_object_set (item,
								"x", x - m_lbearing + m_xHOffs,
								"y", y - m_ascent + m_CHeight + m_yHOffs,
								NULL);
		} else if (n > 0) {
			item = gnome_canvas_item_new (
							group,
							gnome_canvas_pango_get_type(),
							"layout", m_HLayout,
							"x", x - m_lbearing + m_xHOffs,
							"y", y - m_ascent + m_CHeight + m_yHOffs,
							NULL);
			g_object_set_data (G_OBJECT (group), "hydrogens", item);
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
		item = g_object_get_data (G_OBJECT (group), "hydrogens");
		if (item) {
			gtk_object_destroy (GTK_OBJECT (item));
			g_object_set_data ((GObject*)group, "hydrogens", NULL);
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
}*/
	
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
		while (electron) { 
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
	while (electron) { 
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

bool Atom::GetPosition (double angle, double& x, double& y)
{
	if (angle > 360.)
		angle -= 360;
	else if (angle < 0.)
		angle += 360;
	if (((GetZ() == 6) && (m_Bonds.size() != 0)) ||
		 !m_nH || ((m_HPos == LEFT_HPOS && (angle < 135. || angle > 225.)) ||
			(m_HPos == RIGHT_HPOS && (angle > 45. && angle < 315.)) ||
			(m_HPos == TOP_HPOS && (angle > 45. || angle > 135.)) ||
			(m_HPos == BOTTOM_HPOS && (angle < 225. || angle > 315.)))) {
		double t = tan (angle / 180. * M_PI);
		double limit = atan (m_height / m_width) * 180. / M_PI;
		if (angle < limit) {
			x = m_x + m_width / 2.;
			y = m_y - m_width / 2. * t;
		} else if (angle < 180. - limit) {
			if (!isnan (t))
				x = m_x + m_height / 2. / t;
			else
				x = m_x;
			y = m_y - m_height / 2.;
		} else if (angle < 180. + limit) {
			x = m_x - m_width / 2.;
			y = m_y + m_width / 2. * t;
		} else if (angle < 360. - limit) {
			if (!isnan (t))
				x = m_x - m_height / 2. / t;
			else
				x = m_x;
			y = m_y + m_height / 2.;
		} else {
			x = m_x + m_width / 2.;
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
	
xmlNodePtr Atom::Save (xmlDocPtr xml) const
{
	xmlNodePtr node = gcu::Atom::Save (xml), child;
	if (node) {
	// Save electrons
		map<string, Object*>::const_iterator i;
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
			case POSITION_NE:
				buf = "ne";
				break;
			case POSITION_NW:
				buf = "nw";
				break;
			case POSITION_N:
				buf = "n";
				break;
			case POSITION_SE:
				buf = "se";
				break;
			case POSITION_SW:
				buf = "sw";
				break;
			case POSITION_S:
				buf = "s";
				break;
			case POSITION_E:
				buf = "e";
				break;
			case POSITION_W:
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
	if (m_HPosStyle != AUTO_HPOS) {
		char const *pos;
		switch (m_HPosStyle) {
		default:
		case RIGHT_HPOS:
			pos = "right";
			break;
		case LEFT_HPOS:
			pos = "left";
			break;
		case TOP_HPOS:
			pos = "top";
			break;
		case BOTTOM_HPOS:
			pos = "bottom";
			break;
		}
		xmlNewProp (node, reinterpret_cast <xmlChar const*> ("H-position"),
					reinterpret_cast <xmlChar const*> (pos));
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
		else if (strcmp (reinterpret_cast <char const *> (child->name), "position")
		    && strcmp (reinterpret_cast <char const *> (child->name), "text")) {
			Object *obj = CreateObject (reinterpret_cast <char const *> (child->name));
			if (obj) {
				AddChild (obj);
				if (!obj->Load (child))
						return false;
			}
		}
		if (electron && !electron->Load (child))
			return false;
		child = child->next;
	}
	char *buf = (char*) xmlGetProp (node, (xmlChar*) "charge-position");
	m_ChargePos = 0xff;
	if (buf) {
		if (! strcmp (buf, "ne")) {
			m_ChargePos = POSITION_NE;
			m_ChargeAngle = M_PI / 4.;
		} else if (! strcmp (buf, "nw")) {
			m_ChargePos = POSITION_NW;
			m_ChargeAngle = 3. * M_PI / 4.;
		} else if (! strcmp (buf, "n")) {
			m_ChargePos = POSITION_N;
			m_ChargeAngle = M_PI / 2.;
		} else if (! strcmp (buf, "se")) {
			m_ChargePos = POSITION_SE;
			m_ChargeAngle = 7. * M_PI / 4;
		} else if (! strcmp (buf, "sw")) {
			m_ChargePos = POSITION_SW;
			m_ChargeAngle = 5. * M_PI / 4;
		} else if (! strcmp (buf, "s")) {
			m_ChargePos = POSITION_S;
			m_ChargeAngle = 3 * M_PI / 2.;
		} else if (! strcmp (buf, "e")) {
			m_ChargePos = POSITION_E;
			m_ChargeAngle = 0.;
		} else if (! strcmp (buf, "w")) {
			m_ChargePos = POSITION_W;
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
	// Load H atoms position if any
	buf = (char*) xmlGetProp (node, (xmlChar*) "H-position");
	if (buf) {
		if (!strcmp (buf, "left"))
			m_HPosStyle = LEFT_HPOS;
		else if (!strcmp (buf, "right"))
			m_HPosStyle = RIGHT_HPOS;
		else if (!strcmp (buf, "top"))
			m_HPosStyle = TOP_HPOS;
		else if (!strcmp (buf, "bottom"))
			m_HPosStyle = BOTTOM_HPOS;
		else // who know?
			m_HPosStyle = AUTO_HPOS;
		xmlFree (buf);
		Update ();
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

char Atom::GetChargePosition (double *Angle, double *Dist) const
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
	gcu::Atom::Transform2D (m, x, y);
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
	View *view = Doc->GetView ();
	view->Update (pAtom);
	// also update the bonds
	map<gcu::Atom*, gcu::Bond*>::iterator i;
	Bond *bond = static_cast <Bond *> (pAtom->GetFirstBond (i));
	while (bond) {
		bond->SetDirty ();
		view->Update (bond);
		bond = static_cast <Bond *> (pAtom->GetNextBond (i));
	}
	
}

static void do_choose_H_pos (Atom* Atom)
{
	new HPosDlg (static_cast<Document*> (Atom->GetDocument ()), Atom);
}

bool Atom::BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y)
{
	bool result = false;
	GtkActionGroup *group = NULL;
	GtkAction *action;
	if (GetZ () == 6 && GetBondsNumber() != 0) {
		group = gtk_action_group_new ("atom");
		action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		action = GTK_ACTION (gtk_toggle_action_new ("show-symbol", _("Display symbol"),  _("Whether to display carbon atom symbol or not"), NULL));
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), m_ShowSymbol);
		g_signal_connect (action, "toggled", G_CALLBACK (do_display_symbol), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='show-symbol'/></menu></popup></ui>", -1, NULL);
		result = true;
	}
	if (m_nH) {
		if (!group) {
			group = gtk_action_group_new ("atom");
			action = gtk_action_new ("Atom", _("Atom"),NULL, NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
		}
		action = GTK_ACTION (gtk_action_new ("H-position", _("Hydrogen atoms position"),  NULL, NULL));
		g_signal_connect_swapped (action, "activate", G_CALLBACK (do_choose_H_pos), this);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Atom'><menuitem action='H-position'/></menu></popup></ui>", -1, NULL);
	}
	if (group) {
		gtk_ui_manager_insert_action_group (UIManager, group, 0);
		g_object_unref (group);
	}
	return result | Object::BuildContextualMenu (UIManager, object, x, y);
}

bool Atom::Match (gcu::Atom *atom, AtomMatchState &state)
{
	if (m_nH != static_cast <Atom*> (atom)->m_nH)
		return false;
	return gcu::Atom::Match (atom, state);
}

void Atom::GetSymbolGeometry (double &width, double &height, double &angle, bool up) const
{
	if ((GetZ() != 6) || (GetBondsNumber () == 0) || m_ShowSymbol) {
		width = m_SWidth;
		if (up) {
			height = m_SHeightH ;
			angle = m_SAngleH;
		} else {
			height = m_SHeightL ;
			angle = m_SAngleL;
		}
	} else
		width = height  = angle = 0.;
}

void Atom::BuildSymbolGeometry (double width, double height, double ascent)
{
	m_SWidth = width / 2.;
	m_SHeightH = ascent + 1.; // we use ink extent vertically and logical extent horizonatlly
	m_SHeightL = height - m_SHeightH + 2.;
	m_SAngleH = atan2 (m_SHeightH, m_SWidth);
	m_SAngleL = atan2 (m_SHeightL, m_SWidth);
}

void Atom::AddItem ()
{
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= theme->GetZoomFactor ();
	y *= theme->GetZoomFactor ();
	// always use a group, even if not needed
	gccv::Group *group = new gccv::Group (view->GetCanvas ()->GetRoot (), x, y, this);
	view->GetCanvas ()->GetRoot ()->MoveToFront (group);
	if ((GetZ() != 6) || (GetBondsNumber() == 0) || m_ShowSymbol) {
		gccv::Text *text = new gccv::Text (group, 0., 0., this);
		text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
		text->SetPadding (theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetFillColor (0);
		text->SetFontDescription (view->GetPangoFontDesc ());
		text->SetText (GetSymbol ());
		text->SetLineOffset (view->GetCHeight ());
		// build the symbol geometry
		int n = GetAttachedHydrogens ();
		gccv::Rect ink, logical;
		text->GetBounds (&ink, &logical);
		BuildSymbolGeometry (text->GetWidth (), ink.y1 - ink.y0, /*text->GetAscent () - view->GetCHeight ()*/ - ink.y0);		
		m_width = (ink.x1 - ink.x0 + 2 * theme->GetPadding ()) / theme->GetZoomFactor ();
		m_height = (ink.y1 - ink.y0 + 2 * theme->GetPadding ()) / theme->GetZoomFactor ();
		if (n > 0) {
			string hs = "H";
			if (n > 1) {
				char *str = g_strdup_printf ("%d", n);
				hs += str;
				g_free (str);
			}
			text = new gccv::Text (group, 0., 0., this);
			text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
			text->SetPadding (theme->GetPadding ());
			text->SetLineColor (0);
			text->SetLineWidth (0.);
			text->SetFillColor (0);
			text->SetFontDescription (view->GetPangoFontDesc ());
			text->SetText (hs.c_str ());
			if (n >1) {
				gccv::TextTag *tag = new gccv::PositionTextTag (gccv::Subscript, text->GetDefaultFontSize ());
				tag->SetStartIndex (1);
				tag->SetEndIndex (hs.length ());
				text->InsertTextTag (tag);
			}
			text->SetLineOffset (view->GetCHeight ());
			switch (m_HPos) {
			case LEFT_HPOS:
				text->SetAnchor (gccv::AnchorLineEast);
				text->SetPosition (logical.x0, 0.);
				break;
			case RIGHT_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (logical.x1, 0.);
				break;
			case TOP_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (-view->GetHWidth (), ink.y0 - ink.y1 - 2.); // 2. is arbitrary
				break;
			case BOTTOM_HPOS:
				text->SetAnchor (gccv::AnchorLineWest);
				text->SetPosition (-view->GetHWidth (), ink.y1 - ink.y0 + 2.);
				break;
			default:
				g_critical ("This should not happen, please file a bug report");
				break;
			}
		}
	} else {
		gccv::FillItem *fill = new gccv::Rectangle (group,  -3., -3., 6., 6., this);
		fill->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
		fill->SetLineColor (0);
		m_width = m_height = 2 * theme->GetPadding () / theme->GetZoomFactor ();
		if (m_DrawCircle) {
			fill = new gccv::Circle ( group, 0., 0., theme->GetStereoBondWidth () / 2., this);
			fill->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
			fill->SetLineColor (0);
		}
	}
	m_Item = group;
	int charge = GetCharge ();
	if (charge) {
		gccv::Anchor anchor = GetChargePosition (m_ChargePos, m_ChargeAngle * 180 / M_PI, x, y);
		if (m_ChargeDist != 0.) {
			anchor = gccv::AnchorCenter;
			x = m_ChargeDist * cos (m_ChargeAngle);
			y = m_ChargeDist * sin (m_ChargeAngle);
		}
		x -= m_x;
		x *= theme->GetZoomFactor ();
		y -= m_y;
		y *= theme->GetZoomFactor ();
		char* markup = NULL;
		char const *glyph = (charge > 0)? "\xE2\x8a\x95": "\xE2\x8a\x96";
		if (abs (m_Charge) > 1)
			markup = g_strdup_printf ("%d%s", abs (m_Charge), glyph);
		else
			markup = g_strdup (glyph);
		gccv::Text *text = new gccv::Text (group, x, y, NULL);
		text->SetColor ((view->GetData ()->IsSelected (this))? SelectColor: GO_COLOR_BLACK);
		text->SetFillColor (0);
		text->SetPadding (theme->GetPadding ());
		text->SetLineColor (0);
		text->SetLineWidth (0.);
		text->SetAnchor (anchor);
		text->SetFontDescription (view->GetPangoSmallFontDesc ());
		text->SetText (markup);
		m_ChargeItem = text;
		g_free (markup);
	} else
		m_ChargeItem = NULL;
}

bool Atom::HasAvailableElectrons (bool paired)
{
	map<string, Object*>::iterator i;
	Electron* electron = static_cast <Electron *> (GetFirstChild (i));
	if (paired) {
		if (m_nlp)
			return true;
		while (electron) { 
			if (electron->IsPair ())
				return true;
			electron = static_cast <Electron *> (GetNextChild (i));
		}
	} else {
		return (electron) || (m_nlp) || (m_nlu); // TODO: take curved arrows into account
	}
	return false;
}

}	//	namespace gcp
