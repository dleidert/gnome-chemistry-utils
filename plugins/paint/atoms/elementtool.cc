// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * elementtool.cc 
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
#include "elementtool.h"
#include <gcu/element.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/application.h>
#include <gcp/theme.h>
#include <math.h>
#include <string.h>
#include <canvas/gcp-canvas-group.h>

gcpElementTool::gcpElementTool (gcp::Application *App): gcp::Tool (App, "Element")
{
}

gcpElementTool::~gcpElementTool ()
{
}

bool gcpElementTool::OnClicked ()
{
	int CurZ = m_pApp->GetCurZ ();
	if (m_pObject) {
		if (m_pObject->GetType () != AtomType)
			return false; 
		if 	(((gcp::Atom*) m_pObject)->GetTotalBondsNumber () > (int) Element::GetMaxBonds (CurZ))
			return false;
		((gcp::Atom*) m_pObject)->GetCoords (&m_x0, &m_y0);
		m_x0 *= m_dZoomFactor;
		m_y0 *= m_dZoomFactor;
	}
	m_bChanged = true;
	const gchar* symbol = gcu::Element::Symbol (CurZ);
	PangoLayout *pl = pango_layout_new (m_pView->GetPangoContext ());
	pango_layout_set_font_description (pl, m_pView->GetPangoFontDesc ());
	pango_layout_set_text (pl, symbol, strlen (symbol));
	gint width = pango_layout_get_width (pl);
	double padding = m_pView->GetDoc ()->GetTheme ()->GetPadding ();
	m_x1 = m_x0 - (double) width / 2 - padding;
	m_y1 = m_y0 - m_pView->GetFontHeight() / 2 - padding;
	m_x2 = m_x0 + (double) width / 2 + padding;
	m_y2 = m_y0 + m_pView->GetFontHeight () / 2 + padding;
	m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
	gnome_canvas_item_new (
					(GnomeCanvasGroup*) m_pItem,
					gnome_canvas_rect_get_type (),
					"x1", m_x1,
					"y1", m_y1,
					"x2", m_x2,
					"y2", m_y2,
					"fill_color", "white",
					NULL);
	gnome_canvas_item_new (
					(GnomeCanvasGroup*) m_pItem,
					gnome_canvas_pango_get_type (),
					"layout", pl,
					"x", rint (m_x0),
					"y", rint (m_y0),
					"anchor", GTK_ANCHOR_CENTER,
					"fill_color", gcp::AddColor,
					NULL);
	g_object_unref (pl);
	return true;
}

void gcpElementTool::OnDrag ()
{
		if ((m_x > m_x1) && (m_x2 > m_x) && (m_y > m_y1) && (m_y2 > m_y)) {
			if (!m_bChanged) {
				gnome_canvas_item_show (m_pItem);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			gnome_canvas_item_hide (m_pItem);
			m_bChanged = false;
		}
}

void gcpElementTool::OnRelease ()
{
	int CurZ = m_pApp->GetCurZ () ;
	if (m_bChanged) {
		gcp::Document* pDoc = m_pView->GetDoc ();
		if (m_pObject)
		{
			gcp ::Molecule* pMol = (gcp::Molecule*) m_pObject->GetMolecule ();
			gcp::Operation* pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			Object *pObj = m_pObject->GetGroup ();
			pOp->AddObject (pObj, 0);
			Object* parent = m_pObject->GetParent ();
			if (m_nState & GDK_CONTROL_MASK && (parent->GetType () == FragmentType)) {
				//If m_pObject points to an atom inside a fragment, replace the whole fragment.
				gcp::Atom* pAtom = ((gcp::Fragment*) parent)->GetAtom ();
				map<Atom*, Bond*>::iterator i;
				gcp::Bond *pBond = (gcp::Bond*) pAtom->GetFirstBond (i);
				double x, y;
				pAtom->GetCoords (&x, &y);
				gcp::Atom* pNewAtom = new gcp::Atom (CurZ, x, y, 0.);
				pMol->Remove (parent);
				m_pView->Remove (parent);
				m_pView->AddObject (pNewAtom);
				parent->SetParent (NULL);
				pMol->AddAtom (pNewAtom);
				pNewAtom->SetId ((gchar*) pAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pAtom, pNewAtom);
					pNewAtom->AddBond (pBond);
				}
				pNewAtom->Update ();
				m_pView->Update (pNewAtom);
				delete parent;
			} else {
				((gcp::Atom*) m_pObject)->SetZ (CurZ);
				m_pView->Update ((gcp::Atom*) m_pObject);
			}	
			pOp->AddObject (pObj, 1);
		} else {
			gcp::Atom* pAtom = new gcp::Atom (CurZ, m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor, 0);
			gcp::Operation* pOp = pDoc-> GetNewOperation (gcp::GCP_ADD_OPERATION);
			pDoc->AddAtom (pAtom);
			pOp->AddObject (pAtom);
		}
		pDoc->FinishOperation ();
	}
}
