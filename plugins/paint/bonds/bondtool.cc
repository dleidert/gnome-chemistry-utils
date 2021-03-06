// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * bondtool.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bondtool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/fragment.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/hash.h>
#include <gccv/line.h>
#include <gccv/squiggle.h>
#include <gccv/wedge.h>
#include <gcugtk/ui-builder.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;
using namespace gccv;
using namespace std;

gcpBondTool::gcpBondTool (gcp::Application *App, string ToolId, G_GNUC_UNUSED unsigned nPoints): gcp::Tool (App, ToolId)
{
	m_pOp = NULL;
	m_AutoDir = false;
}

gcpBondTool::~gcpBondTool ()
{
}

bool gcpBondTool::OnClicked ()
{
	if (Element::GetMaxBonds (m_pApp->GetCurZ()) < 1)
		return false;
	int i;
	m_pAtom = NULL;
	m_bChanged = false;
	m_dAngle = 0.;
	gcp::Bond* pBond;
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (m_pObject)
	{
		TypeId Id = m_pObject->GetType ();
		switch (Id)
		{
		case BondType:
			pBond = static_cast<gcp::Bond*> (m_pObject);
			if (pBond->IsLocked ())
				return false;
			m_pAtom = static_cast<gcp::Atom*> (pBond->GetAtom (0));
			m_pAtom->GetCoords (&m_x0, &m_y0, NULL);
			m_pAtom = static_cast<gcp::Atom*> (pBond->GetAtom (1));
			m_pAtom->GetCoords (&m_x1, &m_y1, NULL);
			m_x0 *= m_dZoomFactor;
			m_y0 *= m_dZoomFactor;
			m_x1 *= m_dZoomFactor;
			m_y1 *= m_dZoomFactor;
			m_bChanged = true;
			m_pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			m_pOp->AddObject (m_pObjectGroup, 0);
			UpdateBond ();
			return true;
		case FragmentType:
			m_pObject = static_cast <gcp::Fragment *> (m_pObject)->GetAtom ();
		case AtomType:
			if (!((gcp::Atom*) m_pObject)->AcceptNewBonds ())
				return false;
			((gcp::Atom*) m_pObject)->GetCoords (&m_x0, &m_y0, NULL);
			m_x0 *= m_dZoomFactor;
			m_y0 *=  m_dZoomFactor;
			/* search  preferred orientation for new bond */
			i = ((gcp::Atom*) m_pObject)->GetBondsNumber ();
			switch (i) {
				case 0:
					break;
				case 1: {
					map < Bondable *, Bond * >::iterator i;
					gcp::Bond* bond = (gcp::Bond*) ((Atom*) m_pObject)->GetFirstBond (i);
					m_RefAngle = m_dAngle = bond->GetAngle2D ((gcp::Atom*) m_pObject);
					m_dAngle += (((m_nState & GDK_LOCK_MASK  && (!(m_nState & GDK_MOD5_MASK))) ||
								  ((!(m_nState & GDK_LOCK_MASK)) && m_nState & GDK_MOD5_MASK)))?
						pDoc->GetBondAngle (): -pDoc->GetBondAngle ();
					m_AutoDir = true;
					break;
				}
				case 2: {
					double a1, a2;
					map < Bondable *, Bond * >::iterator i;
					gcp::Bond* bond = (gcp::Bond*) ((Atom*) m_pObject)->GetFirstBond (i);
					a1 = bond->GetAngle2D ((gcp::Atom*) m_pObject);
					bond = (gcp::Bond*) ((Atom*) m_pObject)->GetNextBond (i);
					a2 = bond->GetAngle2D ((gcp::Atom*) m_pObject);
					m_dAngle = (a1 + a2) / 2.;
					a2 = fabs (a2 - m_dAngle);
					if (a2 < 90.)
						m_dAngle += 180.;
					if (m_dAngle > 360.)
						m_dAngle -= 360.;
					break;
				}
				default:
					break;
			}
			break;
		default:
			return false;
		}
	}
	double a = m_dAngle * M_PI / 180.;
	m_x1 =  m_x0 + pDoc->GetBondLength () * m_dZoomFactor * cos (a);
	m_y1 =  m_y0 - pDoc->GetBondLength () * m_dZoomFactor * sin (a);
	// TODO: reimplement
	Item *pItem = m_pView->GetCanvas ()->GetItemAt (m_x1, m_y1);
	Object* pObject = NULL;
	if (pItem)
		pObject = dynamic_cast <Object *> (pItem->GetClient ());
	m_pAtom = NULL;
	if (pObject && pObject != m_pObject) {
		if ((pObject->GetType () == BondType) || (pObject->GetType () == FragmentType))
		{
			m_pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
		} else if (pObject->GetType() == AtomType) {
			m_pAtom = (gcp::Atom*)pObject;
		}
	}
	if (m_pAtom) {
		if (m_pObject) {
			Object *group = m_pObject->GetMolecule ()->GetParent ();
			if (group != pDoc) {
				Object *other = m_pAtom->GetMolecule ()->GetParent ();
				if (other != pDoc && group != other)
					return true;
			}
		}
		m_pAtom->GetCoords(&m_x1, &m_y1, NULL);
		m_x1 *= m_dZoomFactor;
		m_y1 *= m_dZoomFactor;
		m_x = m_x1 - m_x0;
		m_y = m_y1 - m_y0;
		m_dAngle = atan(-m_y/m_x) * 90 / 1.570796326794897;
		if (m_x < 0) m_dAngle += 180;
	}
	char tmp[32];
	snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), m_dAngle);
	m_pApp->SetStatusText (tmp);
	Draw ();
	return true;
}

void gcpBondTool::OnDrag ()
{
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Theme *Theme = pDoc->GetTheme ();
	if ((m_pObject) && (m_pObject->GetType () == BondType)) {
		if (((gcp::Bond*) m_pObject)->GetDist (m_x / m_dZoomFactor, m_y / m_dZoomFactor) < (Theme->GetPadding () + Theme->GetBondWidth () / 2) * m_dZoomFactor) {
			if (!m_bChanged) {
				m_Item->SetVisible (true);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			m_Item->SetVisible (false);
			m_bChanged = false;
		}
	} else {
		Item *pItem = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
		Object* pObject = NULL;
		if (pItem) {
			pObject = dynamic_cast <Object *> (pItem->GetClient ());
			if (pObject && (pObject == m_pObject || ((pObject->GetType () == FragmentType) && dynamic_cast<gcp::Fragment*> (pObject)->GetAtom () == m_pObject))) {
				if (!m_AutoDir)
					return;
			} else
				m_AutoDir = false;
		} else
			m_AutoDir = false;
		double dAngle = 0.;
		if (m_AutoDir) {
			dAngle = m_dAngle = m_RefAngle +
						((((m_nState & GDK_LOCK_MASK  && (!(m_nState & GDK_MOD5_MASK))) ||
						((!(m_nState & GDK_LOCK_MASK)) && m_nState & GDK_MOD5_MASK)))?
						pDoc->GetBondAngle (): -pDoc->GetBondAngle ());
			m_x = m_x1 = m_x0 + pDoc->GetBondLength () * m_dZoomFactor * cos (m_dAngle / 180 * M_PI);
			m_y = m_y1 = m_y0 - pDoc->GetBondLength () * m_dZoomFactor * sin (m_dAngle / 180 * M_PI);
			pItem = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
			pObject = NULL;
			if (pItem)
				pObject = dynamic_cast <Object *> (pItem->GetClient ());
		}
		m_pAtom = NULL;
		if (gcp::MergeAtoms && pObject) {
			if (pObject->GetType () == BondType)
				m_pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
			else if (pObject->GetType () == FragmentType)
				m_pAtom = (gcp::Atom*)pObject->GetAtomAt (m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
			else if (pObject->GetType () == AtomType)
				m_pAtom = (gcp::Atom*) pObject;
		}
		if (m_pAtom) {
			if (m_pObject) {
				Object *group = m_pObject->GetMolecule ()->GetParent ();
				if (group != pDoc) {
					Object *other = m_pAtom->GetMolecule ()->GetParent ();
					if (other != pDoc && group != other)
						return;
				}
			}
			if ((Object*) m_pAtom == m_pObject)
				return;
			if (!m_pAtom->AcceptNewBonds ())
				return;
			m_pAtom->GetCoords (&m_x1, &m_y1, NULL);
			m_x1 *= m_dZoomFactor;
			m_y1 *= m_dZoomFactor;
			m_x = m_x1 - m_x0;
			m_y = m_y1 - m_y0;
			dAngle = atan (-m_y / m_x) * 180. / M_PI;
			if (isnan (dAngle))
				dAngle = m_dAngle;
			else if (m_x < 0.)
				dAngle += 180.;
		} else if (!m_AutoDir) {
			m_x -= m_x0;
			m_y -= m_y0;
			if (m_x == 0) {
				if (m_y == 0)
					return;
				dAngle = (m_y < 0) ? 90 : 270;
			} else {
				dAngle = atan (-m_y/m_x) * 180. / M_PI;
				if (!(m_nState & GDK_CONTROL_MASK))
					dAngle = rint (dAngle / 5) * 5;
				if (isnan (dAngle))
					dAngle = m_dAngle;
				else if (m_x < 0.)
					dAngle += 180.;
			}
			m_dAngle = dAngle;
			if (m_nState & GDK_SHIFT_MASK) {
				double x1 = sqrt (square (m_x) + square (m_y));
				m_x1 = m_x0 + x1 * cos (m_dAngle / 180 * M_PI);
				m_y1 = m_y0 - x1 * sin (m_dAngle / 180 * M_PI);
			} else {
				m_x1 = m_x0 + pDoc->GetBondLength () * m_dZoomFactor * cos (m_dAngle / 180 * M_PI);
				m_y1 = m_y0 - pDoc->GetBondLength () * m_dZoomFactor * sin (m_dAngle / 180 * M_PI);
			}
		}
		char tmp[32];
		if (dAngle < 0)
			dAngle += 360.;
		snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), dAngle);
		m_pApp->SetStatusText (tmp);
		Draw ();
	}
}

void gcpBondTool::OnRelease ()
{
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	} else {
		if (m_pOp)
			pDoc->AbortOperation ();
		m_pOp = NULL;
		return;
	}
	if ((m_pObject) && (m_pObject->GetType () == BondType)) {
		FinalizeBond ();
		gcp::Atom* pAtom = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
		pAtom->Update ();
		m_pView->Update (pAtom);
		pAtom = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
		pAtom->Update ();
		m_pView->Update (pAtom);
		m_pOp->AddObject (m_pObjectGroup, 1);
		pDoc->FinishOperation ();
		m_pOp = NULL;
		m_pObject->EmitSignal (gcp::OnChangedSignal);
		return;
	} else {
		if (m_pOp)
			pDoc->AbortOperation();
		m_pOp = NULL;
	}
	m_pApp->ClearStatus ();
	Item *pItem = m_pView->GetCanvas ()->GetItemAt (m_x1, m_y1);
	Object* pObject = NULL;
	if (pItem)
		pObject = dynamic_cast <Object *> (pItem->GetClient ());
	m_pAtom = NULL;
	if (gcp::MergeAtoms && pObject) {
		if (pObject->GetType () == BondType)
			m_pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
		else if (pObject->GetType() == FragmentType)
			m_pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
		else if (pObject->GetType() == AtomType)
			m_pAtom = (gcp::Atom*) pObject;
	}
	gcp::Atom* pAtom;
	gcp::Bond* pBond;
	if (!m_pObject) {
		//Add an atom at (x0, y0)
		pAtom = new gcp::Atom (m_pApp->GetCurZ(), m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor, 0);
		pDoc->AddAtom (pAtom);
		m_pObject = pAtom;
	} else {
		pObject = m_pObject->GetGroup ();
		if (pObject)
			ModifiedObjects.insert (pObject->GetId ());
	}
	if (m_pObject->GetType () == AtomType) {
		if (m_pAtom) {
			if (m_pObject == m_pAtom) {
				ModifiedObjects.clear ();
				return;
			}
			pObject = m_pAtom->GetGroup ();
			if (!pObject)
				throw runtime_error (_("Invalid document tree, please file a bug report"));
			ModifiedObjects.insert (pObject->GetId ());
			pAtom = m_pAtom;
		} else {
			pAtom = new gcp::Atom (m_pApp->GetCurZ (), m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor, 0);
			pDoc->AddAtom (pAtom);
		}
		pBond = (gcp::Bond*) pAtom->GetBond ((gcp::Atom*) m_pObject);
		if (pBond) {
			m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			m_pOp->AddObject (pBond->GetGroup (), 0);
			if (pBond->GetType () == gcp::NormalBondType)
				pBond->IncOrder ();
			m_pObject = pBond;
			m_bChanged = true;
			FinalizeBond ();
			gcp::Atom* pAtom = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (0);
			pAtom->Update ();
			m_pView->Update (pAtom);
			pAtom = (gcp::Atom*) ((gcp::Bond*) m_pObject)->GetAtom (1);
			pAtom->Update ();
			m_pView->Update (pAtom);
			m_pView->Update (pBond);
			m_pOp->AddObject (pBond->GetGroup (), 1);
			pDoc->FinishOperation ();
			m_pOp = NULL;
		} else {
			// Push modified objects in Operation
			if (ModifiedObjects.size ()) {
				m_pOp = pDoc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
				set<string>::iterator it, end = ModifiedObjects.end ();
				for (it = ModifiedObjects.begin (); it != end; it++)
					m_pOp->AddObject (pDoc->GetDescendant ((*it).c_str ()), 0);
			}
			pBond = new gcp::Bond ((gcp::Atom*) m_pObject, pAtom, 1);
			SetType (pBond);
			pDoc->AddBond (pBond);
			if (m_pOp) {
				set<string>::iterator it, end = ModifiedObjects.end ();
				for (it = ModifiedObjects.begin (); it != end; it++) {
					pObject = pDoc->GetDescendant ((*it).c_str ());
					if (pObject)
						m_pOp->AddObject (pObject, 1);
				}
			} else {
				m_pOp = pDoc-> GetNewOperation (gcp::GCP_ADD_OPERATION);
				m_pOp->AddObject (pBond->GetMolecule ());
			}
			pDoc->FinishOperation ();
		}
	}
	ModifiedObjects.clear ();
}

void gcpBondTool::Draw ()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		static_cast <Line *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
	} else {
		m_Item = new gccv::Line (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		static_cast <LineItem *> (m_Item)->SetLineColor (gcp::AddColor);
		static_cast <LineItem *> (m_Item)->SetLineWidth (pTheme->GetBondWidth ());
	}
}

void gcpBondTool::UpdateBond()
{
	double x1, y1, x2, y2;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
	BondOrder = bond->GetOrder ();
	if (bond->GetType () == gcp::NormalBondType)
		bond->IncOrder ();
	if (m_Item)
		delete m_Item;
	if (bond->GetOrder () == 1) {
		bond->GetLine2DCoords (1, &x1, &y1, &x2, &y2);
		m_Item = new gccv::Line (m_pView->GetCanvas (),
								 x1 * m_dZoomFactor, y1 * m_dZoomFactor,
								 x2 * m_dZoomFactor, y2 * m_dZoomFactor);
		static_cast <LineItem *> (m_Item)->SetLineColor (gcp::AddColor);
		static_cast <LineItem *> (m_Item)->SetLineWidth (pTheme->GetBondWidth ());
	} else {
		int i = 1;
		m_Item = new gccv::Group (m_pView->GetCanvas ());
		while (((gcp::Bond*) m_pObject)->GetLine2DCoords (i++, &x1, &y1, &x2, &y2)) {
			gccv::LineItem *item = new gccv::Line (static_cast <gccv::Group *> (m_Item),
												   x1 * m_dZoomFactor, y1 * m_dZoomFactor,
												   x2 * m_dZoomFactor, y2 * m_dZoomFactor);
			item->SetLineColor (gcp::AddColor);
			item->SetLineWidth (pTheme->GetBondWidth ());
		}
	}
}

void gcpBondTool::FinalizeBond ()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		if (pBond->GetType () == gcp::NormalBondType)
			m_pView->Update (m_pObject);
		else {
			pBond->SetType (gcp::NormalBondType);
			m_pView->Update (m_pObject);
		}
	}
	else ((gcp::Bond*) m_pObject)->SetOrder (BondOrder);
	m_pView->Update(((gcp::Bond*) m_pObject)->GetAtom (0));
	m_pView->Update(((gcp::Bond*) m_pObject)->GetAtom (1));
}

void gcpBondTool::SetType (gcp::Bond* pBond)
{
	pBond->SetType (gcp::NormalBondType);
}

static void on_length_changed (GtkSpinButton *btn, gcpBondTool *tool)
{
	tool->SetLength (gtk_spin_button_get_value (btn));
}

static void on_angle_changed (GtkSpinButton *btn, gcpBondTool *tool)
{
	tool->SetAngle (gtk_spin_button_get_value (btn));
}

static void on_merge_toggled (GtkToggleButton *btn)
{
	gcp::MergeAtoms = gtk_toggle_button_get_active (btn);
}

void gcpBondTool::SetAngle (double angle)
{
	m_pApp->GetActiveDocument ()->SetBondAngle (angle);
}

void gcpBondTool::SetLength (double length)
{
	m_pApp->GetActiveDocument ()->SetBondLength (length);
}

GtkWidget *gcpBondTool::GetPropertyPage ()
{
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/bond.ui", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (builder->GetWidget ("bond-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	m_AngleBtn = GTK_SPIN_BUTTON (builder->GetWidget ("bond-angle"));
	g_signal_connect (m_AngleBtn, "value-changed", G_CALLBACK (on_angle_changed), this);
	m_MergeBtn = GTK_TOGGLE_BUTTON (builder->GetWidget ("merge"));
	g_signal_connect (m_MergeBtn, "toggled", G_CALLBACK (on_merge_toggled), NULL);
	GtkWidget *res = builder->GetRefdWidget ("bond");
	delete builder;
	return res;
}

void gcpBondTool::Activate ()
{
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	gtk_spin_button_set_value (m_LengthBtn, pDoc->GetBondLength ());
	gtk_spin_button_set_value (m_AngleBtn, pDoc->GetBondAngle ());
	gtk_toggle_button_set_active (m_MergeBtn, gcp::MergeAtoms);
}

gcpUpBondTool::gcpUpBondTool (gcp::Application *App): gcpBondTool (App, "UpBond", 3)
{
}

gcpUpBondTool::~gcpUpBondTool ()
{
}

void gcpUpBondTool::Draw ()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		static_cast <Wedge *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
	} else {
		m_Item = new Wedge (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1, pTheme->GetStereoBondWidth ());
		static_cast <Wedge *> (m_Item)->SetFillColor (gcp::AddColor);
	}
}

void gcpUpBondTool::UpdateBond ()
{
	if (((gcp::Bond*) m_pObject)->GetType () == gcp::UpBondType) {
		m_x = m_x0;
		m_x0 = m_x1;
		m_x1 = m_x;
		m_y = m_y0;
		m_y0 = m_y1;
		m_y1 = m_y;
	}
	Draw ();
}

void gcpUpBondTool::FinalizeBond()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		if (pBond->GetType () == gcp::UpBondType) {
			pBond->Revert ();
			m_pView->Update (m_pObject);
		} else {
			pBond->SetType (gcp::UpBondType);
			m_pView->Remove (m_pObject);
			m_pView->AddObject (m_pObject);
		}
	}
}

void gcpUpBondTool::SetType (gcp::Bond* pBond)//FIXME: Is it really useful?
{
	pBond->SetType (gcp::UpBondType);
}

static void on_config_changed (GOConfNode *node, G_GNUC_UNUSED gchar const *key, gcpDownBondTool *tool)
{
	tool->UpdateItem (go_conf_get_bool (node, "invert-wedge-hashes"));
}

gcpDownBondTool::gcpDownBondTool (gcp::Application *App, gccv::Wedge *item):
	gcpBondTool (App, "DownBond", 4),
	m_Wedge (item)
{
	m_ConfNode = go_conf_get_node (App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
	m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) on_config_changed, this);
}

gcpDownBondTool::~gcpDownBondTool ()
{
	go_conf_remove_monitor (m_NotificationId);
	go_conf_free_node (m_ConfNode);
}

void gcpDownBondTool::UpdateItem (bool inverted)
{
	if (inverted)
		m_Wedge->SetPosition (2., 22., 19., 5.);
	else
		m_Wedge->SetPosition (19., 5., 2., 22.);
}

void gcpDownBondTool::Draw()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		if (gcp::InvertWedgeHashes)
			static_cast <Hash *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
		else
			static_cast <Hash *> (m_Item)->SetPosition (m_x1, m_y1, m_x0, m_y0);
	} else {
		gccv::Hash *hash = gcp::InvertWedgeHashes?
			new Hash (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1, pTheme->GetStereoBondWidth ()):
			new Hash (m_pView->GetCanvas (), m_x1, m_y1, m_x0, m_y0, pTheme->GetStereoBondWidth ());
		hash->SetFillColor (gcp::AddColor);
		hash->SetLineWidth (pTheme->GetHashWidth ());
		hash->SetLineDist (pTheme->GetHashDist ());
		m_Item = hash;
	}
}

void gcpDownBondTool::UpdateBond ()
{
	if (((gcp::Bond*) m_pObject)->GetType() == gcp::DownBondType) {
		m_x = m_x0;
		m_x0 = m_x1;
		m_x1 = m_x;
		m_y = m_y0;
		m_y0 = m_y1;
		m_y1 = m_y;
	}
	Draw ();
}

void gcpDownBondTool::FinalizeBond ()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*)m_pObject;
		if (pBond->GetType () == gcp::DownBondType) {
			pBond->Revert ();
			m_pView->Update (m_pObject);
		} else {
			pBond->SetType (gcp::DownBondType);
			m_pView->Remove (m_pObject);
			m_pView->AddObject (m_pObject);
		}
	}
}

void gcpDownBondTool::SetType (gcp::Bond* pBond)
{
	pBond->SetType (gcp::DownBondType);
}

gcpForeBondTool::gcpForeBondTool (gcp::Application *App): gcpBondTool (App, "ForeBond", 4)
{
}

gcpForeBondTool::~gcpForeBondTool ()
{
}

void gcpForeBondTool::Draw ()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		static_cast <Line *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
	} else {
		m_Item = new gccv::Line (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		static_cast <LineItem *> (m_Item)->SetLineColor (gcp::AddColor);
		static_cast <LineItem *> (m_Item)->SetLineWidth (pTheme->GetStereoBondWidth ());
	}
}

void gcpForeBondTool::UpdateBond ()
{
	Draw ();
}

void gcpForeBondTool::FinalizeBond ()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		if (pBond->GetType () != gcp::ForeBondType) {
			pBond->SetType (gcp::ForeBondType);
			m_pView->Remove (m_pObject);
			m_pView->AddObject (m_pObject);
		}
	}
}

void gcpForeBondTool::SetType (gcp::Bond* pBond)
{
	pBond->SetType (gcp::ForeBondType);
}

gcpSquiggleBondTool::gcpSquiggleBondTool (gcp::Application *App): gcpBondTool (App, "SquiggleBond", 4)
{
}

gcpSquiggleBondTool::~gcpSquiggleBondTool ()
{
}

void gcpSquiggleBondTool::Draw ()
{
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		static_cast <Squiggle *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
	} else {
		Squiggle *squiggle = new Squiggle (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		squiggle->SetLineColor (gcp::AddColor);
		squiggle->SetLineWidth (pTheme->GetBondWidth ());
		squiggle->SetWidth (pTheme->GetStereoBondWidth () - pTheme->GetBondWidth () / 2.);
		squiggle->SetStep (pTheme->GetStereoBondWidth () / 2.);
		m_Item = squiggle;
	}
}

void gcpSquiggleBondTool::UpdateBond ()
{
	Draw ();
}

void gcpSquiggleBondTool::FinalizeBond ()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		if (pBond->GetType () == gcp::UndeterminedBondType) {
			pBond->Revert ();
			m_pView->Update (m_pObject);
		} else {
			pBond->SetType (gcp::UndeterminedBondType);
			m_pView->Remove (m_pObject);
			m_pView->AddObject (m_pObject);
		}
	}
}

void gcpSquiggleBondTool::SetType (gcp::Bond* pBond)
{
	pBond->SetType (gcp::UndeterminedBondType);
}

gcpWeakBondTool::gcpWeakBondTool (gcp::Application *App): gcpBondTool (App, "WeakBond")
{
}

gcpWeakBondTool::~gcpWeakBondTool ()
{
}

void gcpWeakBondTool::Draw ()
{
	gcp::Theme *theme = m_pView->GetDoc ()->GetTheme ();
	if (m_Item) {
		static_cast <gccv::Line *> (m_Item)->SetPosition (m_x0, m_y0, m_x1, m_y1);
	} else {
		m_Item = new gccv::Line (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		gccv::Line *line = static_cast <gccv::Line *> (m_Item);
		double dashes[] = {3., 2.};
		line->SetLineColor (gcp::AddColor);
		line->SetDashes (dashes, 2, 0.);
		line->SetLineWidth (theme->GetBondWidth () / 2.);
	}
}

void gcpWeakBondTool::UpdateBond ()
{
	Draw ();
}

void gcpWeakBondTool::FinalizeBond()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		pBond->SetType (gcp::WeakBondType);
		m_pView->Remove (m_pObject);
		m_pView->AddObject (m_pObject);
	}
}

void gcpWeakBondTool::SetType (gcp::Bond* pBond)//FIXME: Is it really useful?
{
	pBond->SetType (gcp::WeakBondType);
}
