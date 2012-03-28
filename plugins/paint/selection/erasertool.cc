// -*- C++ -*-

/*
 * GChemPaint selection plugin
 * erasertool.cc
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
#include "erasertool.h"
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/item.h>
#include <gccv/item-client.h>
#include <cmath>

using namespace gcu;

gcpEraserTool::gcpEraserTool (gcp::Application* App): gcp::Tool (App, "Erase")
{
	m_bChanged = false;
}

gcpEraserTool::~gcpEraserTool ()
{
}

bool gcpEraserTool::OnClicked ()
{
	m_pData->UnselectAll ();
	if (m_pObject) {
		if (m_pObject->IsLocked ())
			return false;
		TypeId Id = m_pObject->GetType ();
		if (Id == ReactionOperatorType)
			return false; //It's an automatic object, don't delete it
		gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (m_pObject);
		if (client) {
			client->SetSelected (gcp::SelStateErasing);
			m_Item = client->GetItem ();
		}
		if (Id == AtomType) {
			Object* parent = m_pObject->GetParent ();
			if (parent->GetType () == FragmentType)
				m_Item = dynamic_cast <gccv::ItemClient *> (parent)->GetItem ();
			std::map<Atom*, Bond*>::iterator i;
			gcp::Bond* pBond = (gcp::Bond*)((gcp::Atom*)m_pObject)->GetFirstBond (i);
			while (pBond) {
				pBond->SetSelected (gcp::SelStateErasing);
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
	gccv::Item *pItem = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
	Object* pObject = NULL;
	gccv::ItemClient *client = dynamic_cast <gccv::ItemClient *> (m_pObject);
	switch (Id) {
	case BondType:
		if (((gcp::Bond*) m_pObject)->GetDist (m_x / m_dZoomFactor, m_y / m_dZoomFactor) < (Theme->GetPadding () + Theme->GetBondWidth () / 2) / m_dZoomFactor) {
			if (!m_bChanged) {
				client->SetSelected (gcp::SelStateErasing);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			client->SetSelected (gcp::SelStateUnselected);
			m_bChanged = false;
		}
		break;
	case AtomType:
		if (pItem) {
			gccv::ItemClient *cli = pItem->GetClient ();
			pObject = (cli)? dynamic_cast <Object *> (cli): NULL;
		}
		if (pObject) {
			if (pObject->GetType () == BondType)
				pObject = ((gcp::Bond*) pObject)->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
			else if (pObject->GetType () == FragmentType)
				pObject = ((gcp::Fragment*) pObject)->GetAtom ();
		}
		if (pObject == m_pObject) {
			if (!m_bChanged) {
				client->SetSelected (gcp::SelStateErasing);
				std::map<Atom*, Bond*>::iterator i;
				gcp::Bond* pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetFirstBond (i);
				while (pBond) {
					pBond->SetSelected (gcp::SelStateErasing);
					pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
				}
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			client->SetSelected (gcp::SelStateUnselected);
			std::map<Atom*, Bond*>::iterator i;
			gcp::Bond* pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetFirstBond (i);
			while (pBond) {
				pBond->SetSelected (gcp::SelStateUnselected);
				pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
			}
			m_bChanged = false;
		}
		break;
	default:
		if (pItem) {
			gccv::ItemClient *cli = pItem->GetClient ();
			pObject = (cli)? dynamic_cast <Object *> (cli): NULL;
		}
		if (pObject == m_pObject) {
			if (!m_bChanged) {
				client->SetSelected (gcp::SelStateErasing);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			client->SetSelected (gcp::SelStateUnselected);
			m_bChanged = false;
		}
	}
}

void gcpEraserTool::OnRelease ()
{
	char *id = NULL;
	if ((!m_pObject) || (!m_bChanged)) {
		m_Item = NULL;
		return;
	}
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Operation *pOp;
	Object *pObj = m_pObject->GetGroup (), *Parent;
	if (m_pObject->GetType () == AtomType) {
		Object* parent = m_pObject->GetParent ();
		if (parent->GetType () == FragmentType)
			m_pObject = parent;
	}
	Parent = m_pObject->GetParent ();
	if (pObj) {
		pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
		pOp->AddObject (pObj, 0);
		id = g_strdup (pObj->GetId ());
	} else {
		pOp = pDoc->GetNewOperation (gcp::GCP_DELETE_OPERATION);
		pOp->AddObject (m_pObject);
	}
// A molecule might disappear, so get its parent
	if (Parent->GetType () == MoleculeType)
		Parent = Parent->GetParent ();
	m_Item = NULL;
	pDoc->Remove (m_pObject);
	Parent->EmitSignal (gcp::OnChangedSignal);
	if (id) {
		pObj = pDoc->GetChild (id);
		if (pObj)
			pOp->AddObject (pObj, 1);
		std::set <std::string> &NewObjects = pDoc->GetNewObjects ();
		std::set <std::string>::iterator i, end = NewObjects.end ();
		for (i = NewObjects.begin (); i != end; i++)
			if (*i != id) {
				pObj = pDoc->GetChild ((*i).c_str ());
				if (pObj)
					pOp->AddObject (pObj, 1);
			}
		g_free (id);
	}
	pDoc->FinishOperation ();
}

void gcpEraserTool::OnMotion ()
{
	m_pData->UnselectAll ();
	if (m_pObject) {
		if (m_pObject->IsLocked ())
			return;
		TypeId Id = m_pObject->GetType ();
		if (Id == ReactionOperatorType)
			return; //It's an automatic object, can't be deleted
		m_pData->SetSelected (m_pObject, gcp::SelStateErasing);
		if (Id == AtomType) {
			std::map<Atom*, Bond*>::iterator i;
			gcp::Bond* pBond = (gcp::Bond*)((gcp::Atom*)m_pObject)->GetFirstBond (i);
			while (pBond) {
				m_pData->SetSelected (pBond, gcp::SelStateErasing);
				pBond = (gcp::Bond*) ((gcp::Atom*) m_pObject)->GetNextBond (i);
			}
		}
	}
}

void gcpEraserTool::OnLeaveNotify ()
{
	m_pData->UnselectAll ();
}
