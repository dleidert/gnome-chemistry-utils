// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * erasertool.cc
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
#include "erasertool.h"
#include <gcp/settings.h>
#include <gcp/document.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/theme.h>
#include <cmath>

gcpEraserTool::gcpEraserTool (gcp::Application* App): gcp::Tool (App, "Erase")
{
	m_bChanged = false;
}

gcpEraserTool::~gcpEraserTool ()
{
}

bool gcpEraserTool::OnClicked ()
{
	if (m_pObject) {
		TypeId Id = m_pObject->GetType ();
		if (Id == ReactionOperatorType)
			return false; //It's an automatic object, don't delete it
		m_pObject->SetSelected (m_pWidget, gcp::SelStateErasing);
		m_pItem = m_pView->GetCanvasItem (m_pWidget, m_pObject);
		if (Id == AtomType) {
			Object* parent = m_pObject->GetParent ();
			if (parent->GetType () == FragmentType)
				m_pItem = m_pView->GetCanvasItem (m_pWidget, parent);
			std::map<Atom*, Bond*>::iterator i;
			gcp::Bond* pBond = (gcp::Bond*)((gcp::Atom*)m_pObject)->GetFirstBond (i);
			while (pBond) {
				pBond->SetSelected (m_pWidget, gcp::SelStateErasing);
				pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
			}
		}
		m_bChanged = true;
		return true;
	}
	return false;
}

void gcpEraserTool::OnDrag ()
{
	if (!m_pObject)
		return;
	TypeId Id = m_pObject->GetType ();
	GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x, m_y);
	gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
	Object* pObject = NULL;
	switch (Id) {
	case BondType:
		if (((gcp::Bond*) m_pObject)->GetDist (m_x / m_dZoomFactor, m_y / m_dZoomFactor) < (Theme->GetPadding () + Theme->GetBondWidth () / 2) / m_dZoomFactor) {
			if (!m_bChanged) {
				m_pObject->SetSelected (m_pWidget, gcp::SelStateErasing);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			m_pObject->SetSelected (m_pWidget, gcp::SelStateUnselected);
			m_bChanged = false;
		}
		break;
	case AtomType:
		if (pItem == (GnomeCanvasItem*) m_pBackground)
			pItem = NULL;
		if (pItem)
			pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
		if (pObject) {
			if (pObject->GetType () == BondType)
				pObject = ((gcp::Bond*) pObject)->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
			else if (pObject->GetType () == FragmentType)
				pObject = ((gcp::Fragment*) pObject)->GetAtom ();
		}
		if (pObject == m_pObject) {
			if (!m_bChanged) {
				m_pObject->SetSelected (m_pWidget, gcp::SelStateErasing);
				std::map<Atom*, Bond*>::iterator i;
				gcp::Bond* pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetFirstBond (i);
				while (pBond) {
					pBond->SetSelected (m_pWidget, gcp::SelStateErasing);
					pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
				}
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			m_pObject->SetSelected (m_pWidget, gcp::SelStateUnselected);
			std::map<Atom*, Bond*>::iterator i;
			gcp::Bond* pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetFirstBond (i);
			while (pBond) {
				pBond->SetSelected (m_pWidget, gcp::SelStateUnselected);
				pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
			}
			m_bChanged = false;
		}
		break;
	default:
		if (pItem)
			pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
		if (pObject == m_pObject) {
			if (!m_bChanged) {
				m_pObject->SetSelected (m_pWidget, gcp::SelStateErasing);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			m_pObject->SetSelected (m_pWidget, gcp::SelStateUnselected);
			m_bChanged = false;
		}
	}
}

void gcpEraserTool::OnRelease ()
{
	char *id = NULL;
	if ((!m_pObject) || (!m_bChanged)) {
		m_pItem = NULL;
		return;
	}
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Operation *pOp;
	Object *pObj = m_pObject->GetGroup (), *Parent;
	if (pObj && (pObj->GetType () != MoleculeType || m_pObject->GetType () == OtherType)) {
		pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		pOp->AddObject (pObj, 0);
		id = g_strdup (pObj->GetId ());
	} else {
		pOp = pDoc->GetNewOperation (gcp::GCP_DELETE_OPERATION);
		pOp->AddObject (m_pObject);
	}
	if (m_pObject->GetType () == AtomType) {
		Object* parent = m_pObject->GetParent ();
		if (parent->GetType () == FragmentType)
			m_pObject = parent;
	}
	Parent = m_pObject->GetParent ();
// A molecule might disappear, so get its parent
	if (Parent->GetType () == MoleculeType)
		Parent = Parent->GetParent ();
	m_pItem = NULL;
	if (m_pData->Items[m_pObject] == NULL) {
		m_pData->Items.erase (m_pObject);
		return;
	}
	pDoc->Remove (m_pObject);
	Parent->EmitSignal (gcp::OnChangedSignal);
	if (id) {
		pObj = pDoc->GetChild (id);
		if (pObj)
			pOp->AddObject (pObj, 1);
		g_free (id);
	}
	pDoc->FinishOperation ();
}
