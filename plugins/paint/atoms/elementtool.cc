// -*- C++ -*-

/*
 * GChemPaint atoms plugin
 * elementtool.cc
 *
 * Copyright (C) 2001-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "elementtool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcu/element.h>
#include <gccv/text.h>
#include <cmath>
#include <cstring>

using namespace gcu;
using namespace std;

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
	gccv::Text *text = new gccv::Text (m_pView->GetCanvas (), m_x0, m_y0);
	m_Item = text;
	const gchar* symbol = gcu::Element::Symbol (CurZ);
	text->SetText (symbol);
	text->SetFontDescription (m_pView->GetPangoFontDesc ());
	text->SetPadding (m_pView->GetDoc ()->GetTheme ()->GetPadding ());
	text->SetFillColor (0);
	text->SetColor (gcp::AddColor);
	text->SetLineOffset (m_pView->GetCHeight ());
	text->GetPosition (m_x1, m_y1, m_x2, m_y2);
	m_x2 += m_x1;
	m_y2 += m_y1;
	return true;
}

void gcpElementTool::OnDrag ()
{
		if ((m_x > m_x1) && (m_x2 > m_x) && (m_y > m_y1) && (m_y2 > m_y)) {
			if (!m_bChanged) {
				m_Item->SetVisible (true);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			m_Item->SetVisible (false);
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
				map < Bondable *, Bond * >::iterator i;
				gcp::Bond *pBond = (gcp::Bond*) pAtom->GetFirstBond (i);
				double x, y;
				pAtom->GetCoords (&x, &y);
				gcp::Atom* pNewAtom = new gcp::Atom (CurZ, x, y, 0.);
				pMol->Remove (parent);
				m_pView->Remove (parent);
				pMol->AddAtom (pNewAtom);
				parent->SetParent (NULL);
				pNewAtom->SetId ((gchar*) pAtom->GetId ());
				if (pBond) {
					pBond->ReplaceAtom (pAtom, pNewAtom);
					pNewAtom->AddBond (pBond);
					pBond->SetDirty ();
					m_pView->Update (pBond);
				}
				pNewAtom->Update ();
				m_pView->AddObject (pNewAtom);
				delete parent;
			} else {
				gcp::Atom* pAtom = static_cast <gcp::Atom *> (m_pObject);
				pAtom->SetZ (CurZ);
				m_pView->Update ((gcp::Atom*) m_pObject);
				map < Bondable *, Bond * >::iterator i;
				gcp::Bond *pBond = (gcp::Bond*) pAtom->GetFirstBond (i);
				while (pBond) {
					pBond->SetDirty ();
					m_pView->Update (pBond);
					pBond = (gcp::Bond*) pAtom->GetNextBond (i);
				}
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
