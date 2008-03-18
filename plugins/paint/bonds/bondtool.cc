// -*- C++ -*-

/* 
 * GChemPaint bonds plugin
 * bondtool.cc
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
#include "bondtool.h"
#include <gcp/settings.h>
#include <gcp/document.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/fragment.h>
#include <gcp/theme.h>
#include <canvas/gcp-canvas-group.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;
using namespace std;

gcpBondTool::gcpBondTool (gcp::Application *App, string ToolId, unsigned nPoints): gcp::Tool (App, ToolId)
{
	points = (nPoints)? gnome_canvas_points_new (nPoints): NULL;
	m_pOp = NULL;
	m_AutoDir = false;
}

gcpBondTool::~gcpBondTool ()
{
	if (points) gnome_canvas_points_free (points);
}

bool gcpBondTool::OnClicked ()
{
	if (Element::GetMaxBonds (m_pApp->GetCurZ()) < 1)
		return false;
	int i;
	m_pAtom = NULL;
	m_pItem = NULL;
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
			points->coords[0] = m_x0;
			points->coords[1] = m_y0;
			m_bChanged = true;
			m_pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			m_pOp->AddObject (m_pObjectGroup, 0);
			UpdateBond ();
			return true;
		case AtomType:
			if (!((gcp::Atom*) m_pObject)->AcceptNewBonds ())
				return false;
			((gcp::Atom*) m_pObject)->GetCoords (&m_x0, &m_y0, NULL);
			m_x0 *= m_dZoomFactor;
			m_y0 *=  m_dZoomFactor;
			points->coords[0] = m_x0;
			points->coords[1] = m_y0;
			/* search  preferred orientation for new bond */
			i = ((gcp::Atom*) m_pObject)->GetBondsNumber ();
			switch (i) {
				case 0:
					break;
				case 1: {
					map<Atom*, Bond*>::iterator i;
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
					map<Atom*, Bond*>::iterator i;
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
	else if (points)
	{
		points->coords[0] = m_x0;
		points->coords[1] = m_y0;
	}
	double a = m_dAngle * M_PI / 180.;
	m_x1 =  m_x0 + pDoc->GetBondLength () * m_dZoomFactor * cos (a);
	m_y1 =  m_y0 - pDoc->GetBondLength () * m_dZoomFactor * sin (a);
	GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x1, m_y1);
	if (pItem == (GnomeCanvasItem*) m_pBackground)
		pItem = NULL;
	Object* pObject = NULL;
	if (pItem)
		pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
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
	double x1, y1, x2, y2;
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Theme *Theme = pDoc->GetTheme ();
	if ((m_pObject) && (m_pObject->GetType () == BondType)) {
		if (((gcp::Bond*) m_pObject)->GetDist (m_x / m_dZoomFactor, m_y / m_dZoomFactor) < (Theme->GetPadding () + Theme->GetBondWidth () / 2) * m_dZoomFactor) {
			if (!m_bChanged) {
				gnome_canvas_item_show (m_pItem);
				m_bChanged = true;
			}
		} else if (m_bChanged) {
			gnome_canvas_item_hide (m_pItem);
			m_bChanged = false;
		}
	} else {
		if (m_pItem) {
			gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
			gtk_object_destroy (GTK_OBJECT (GNOME_CANVAS_ITEM (m_pItem)));
			gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
			m_pItem = NULL;
		}

		GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x, m_y);
		if (pItem == (GnomeCanvasItem*) m_pBackground)
			pItem = NULL;
		Object* pObject = NULL;
		if (pItem) {
			pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
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
			pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x, m_y);
			if (pItem == (GnomeCanvasItem*) m_pBackground)
				pItem = NULL;
			pObject = NULL;
			if (pItem)
				pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
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
				x1 = sqrt (square (m_x) + square (m_y));
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
	double x1, y1, x2, y2;
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (m_pItem) {
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
		gtk_object_destroy (GTK_OBJECT(GNOME_CANVAS_ITEM (m_pItem)));
		gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
		m_pItem = NULL;
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
	GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x1, m_y1);
	if (pItem == (GnomeCanvasItem*) m_pBackground)
		pItem = NULL;
	Object* pObject = NULL;
	if (pItem)
		pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
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
			pObject = m_pObject->GetGroup ();
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
	double x1, y1, x2, y2;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	points->coords[2] = m_x1;
	points->coords[3] = m_y1;
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_line_get_type (),
								"points", points,
								"fill_color", gcp::AddColor,
								"width_units", pTheme->GetBondWidth (),
								NULL);
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
	gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
}

void gcpBondTool::UpdateBond()
{
	double x1, y1, x2, y2;
	int i = 1;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	BondOrder = ((gcp::Bond*) m_pObject)->GetOrder ();
	if (((gcp::Bond*) m_pObject)->GetType () == gcp::NormalBondType)
		((gcp::Bond*) m_pObject)->IncOrder ();
	m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
	while (((gcp::Bond*) m_pObject)->GetLine2DCoords (i++, &x1, &y1, &x2, &y2)) {
		points->coords[0] = x1 * m_dZoomFactor;
		points->coords[1] = y1 * m_dZoomFactor;
		points->coords[2] = x2 * m_dZoomFactor;
		points->coords[3] = y2 * m_dZoomFactor;
		gnome_canvas_item_new (
						GNOME_CANVAS_GROUP (m_pItem),
						gnome_canvas_line_get_type (),
						"points", points,
						"fill_color", gcp::AddColor,
						"width_units", pTheme->GetBondWidth (),
						NULL);
	}
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
	gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
}

void gcpBondTool::FinalizeBond ()
{
	if (m_bChanged) {
		gcp::Bond* pBond = (gcp::Bond*) m_pObject;
		if (pBond->GetType () == gcp::NormalBondType) {
			pBond->Revert ();
			m_pView->Update (m_pObject);
		} else {
			pBond->SetType (gcp::NormalBondType);
			m_pView->Remove (m_pObject);
			m_pView->AddObject (m_pObject);
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
	GladeXML *xml = glade_xml_new (GLADEDIR"/bond.glade", "bond", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bond-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	m_AngleBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bond-angle"));
	g_signal_connect (m_AngleBtn, "value-changed", G_CALLBACK (on_angle_changed), this);
	m_MergeBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "merge"));
	g_signal_connect (m_MergeBtn, "toggled", G_CALLBACK (on_merge_toggled), NULL);
	return glade_xml_get_widget (xml, "bond");
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
	double dx, dy, x1, y1, x2, y2;
	gcp::Theme *Theme = m_pView->GetDoc ()->GetTheme ();
	x1 = sqrt (square (m_x1 - m_x0) + square (m_y1 - m_y0));
	if (x1 == 0)
		return;
	dx = (m_y0 - m_y1) / x1 * Theme->GetStereoBondWidth () / 2;
	dy = (m_x1 - m_x0) / x1 * Theme->GetStereoBondWidth () / 2;
	points->coords[2] = m_x1 + dx;
	points->coords[3] = m_y1 + dy;
	points->coords[4] = m_x1 - dx;
	points->coords[5] = m_y1 - dy;
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_polygon_get_type (),
								"points", points,
								"fill_color", gcp::AddColor,
								NULL);
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
	gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
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
		points->coords[0] = m_x0;
		points->coords[1] = m_y0;
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

gcpDownBondTool::gcpDownBondTool (gcp::Application *App): gcpBondTool (App, "DownBond", 4)
{
}

gcpDownBondTool::~gcpDownBondTool ()
{
}

void gcpDownBondTool::Draw()
{
	double dx, dy, dx1, dy1, length;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
	length = sqrt (square (m_x1 - m_x0) + square (m_y1 - m_y0));
	if (length == 0.0)
		return;
	int n = int (floor (length / (pTheme->GetHashDist () + pTheme->GetHashWidth ())));
	dx1 = (m_x1 - m_x0) / length * pTheme->GetHashWidth ();
	dy1 = (m_y1 - m_y0) / length * pTheme->GetHashWidth ();
	dx = (m_y0 - m_y1) / length * pTheme->GetStereoBondWidth () / 2;
	dy = (m_x1 - m_x0) / length * pTheme->GetStereoBondWidth () / 2;
	points->coords[0] = m_x0 + dx;
	points->coords[1] = m_y0 + dy;
	points->coords[2] = m_x0 - dx;
	points->coords[3] = m_y0 - dy;
	dx *= (1 - pTheme->GetHashWidth () / length);
	dy *= (1 - pTheme->GetHashWidth () / length);
	points->coords[4] = m_x0 + dx1 - dx;
	points->coords[5] = m_y0 + dy1 - dy;
	points->coords[6] = m_x0 + dx1 + dx;
	points->coords[7] = m_y0 + dy1 + dy;
	dx = (m_x1 - m_x0) / length * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
		- (m_y0 - m_y1) / length * pTheme->GetStereoBondWidth () / 2 * (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / length;
	dy = (m_y1 - m_y0) / length * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
		- (m_x1 - m_x0) / length * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / length;
	dx1 = (m_x1 - m_x0) / length * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
		+ (m_y0 - m_y1) / length * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / length;
	dy1 = (m_y1 - m_y0) / length * (pTheme->GetHashDist () + pTheme->GetHashWidth ())
		+ (m_x1 - m_x0) / length * pTheme->GetStereoBondWidth () / 2 *  (pTheme->GetHashDist () + pTheme->GetHashWidth ()) / length;
	gnome_canvas_item_new (
						GNOME_CANVAS_GROUP (m_pItem),
						gnome_canvas_polygon_get_type (),
						"points", points,
						"fill_color", gcp::AddColor,
						NULL);
	for (int i = 1; i < n; i++) {
		points->coords[0] += dx;
		points->coords[1] += dy;
		points->coords[2] += dx1;
		points->coords[3] += dy1;
		points->coords[6] += dx;
		points->coords[7] += dy;
		points->coords[4] += dx1;
		points->coords[5] += dy1;
		gnome_canvas_item_new (
						GNOME_CANVAS_GROUP (m_pItem),
						gnome_canvas_polygon_get_type (),
						"points", points,
						"fill_color", gcp::AddColor,
						NULL);
	}
	gnome_canvas_item_get_bounds (m_pItem, &dx, &dy, &dx1, &dy1);
	gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) dx, (int) dy, (int) dx1, (int) dy1);
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
	double dx, dy, x1, y1, x2, y2;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	x1 = sqrt (square (m_x1 - m_x0) + square (m_y1 - m_y0));
	if (x1 == 0)
		return;
	dx = (m_y0 - m_y1) / x1 * pTheme->GetStereoBondWidth () / 2;
	dy = (m_x1 - m_x0) / x1 * pTheme->GetStereoBondWidth () / 2;
	points->coords[0] = m_x0 + dx;
	points->coords[1] = m_y0 + dy;
	points->coords[2] = m_x1 + dx;
	points->coords[3] = m_y1 + dy;
	points->coords[4] = m_x1 - dx;
	points->coords[5] = m_y1 - dy;
	points->coords[6] = m_x0 - dx;
	points->coords[7] = m_y0 - dy;
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_polygon_get_type (),
								"points", points,
								"fill_color", gcp::AddColor,
								NULL);
	gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
	gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
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
	GnomeCanvasPathDef *path_def;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	path_def = gnome_canvas_path_def_new ();
	gnome_canvas_path_def_moveto (path_def, m_x0, m_y0);
	double x = m_x0, y = m_y0, dx, dy, length, x1, x2, y1, y2;
	length = sqrt (square (m_x1 - m_x0) + square (m_y1 - m_y0));
	int n = (int)length / 3, s = 1;
	dx = (m_x1 - m_x0) / n;
	dy = (m_y1 - m_y0) / n;
	for (int i = 1; i < n; i++) {
		x1 = x + dx / 3 + dy /1.5 * s;
		y1 = y + dy / 3 - dx /1.5 * s;
		x2 = x + dx / 1.5 + dy /1.5 * s;
		y2 = y + dy / 1.5 - dx /1.5 * s;
		x += dx;
		y += dy;
		s *= -1;
		gnome_canvas_path_def_curveto (path_def, x1, y1, x2, y2, x, y);
	}
	x1 = x + dx / 3 + dy /1.5 * s;
	y1 = y + dy / 3 - dx /1.5 * s;
	x2 = x + dx / 1.5 + dy /1.5 * s;
	y2 = y + dy / 1.5 - dx /1.5 * s;
	gnome_canvas_path_def_curveto (path_def, x1, y1, x2, y2, m_x1, m_y1);
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_bpath_get_type (),
								"outline_color", gcp::AddColor,
								"width_units", pTheme->GetBondWidth (),
								"bpath", path_def,
								NULL);
	gnome_canvas_path_def_unref (path_def);
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
