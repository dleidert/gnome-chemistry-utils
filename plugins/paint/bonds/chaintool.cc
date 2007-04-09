// -*- C++ -*-

/* 
 * GChemPaint bonds plugin
 * chaintool.cc
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "chaintool.h"
#include <gcp/settings.h>
#include <gcp/document.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/theme.h>
#include <glib/gi18n-lib.h>
#include <cmath>

gcpChainTool::gcpChainTool (gcp::Application *App): gcp::Tool (App, "Chain")
{
	m_Length = 0; // < 2 is auto.
	m_Points = gnome_canvas_points_new (3);
	m_Atoms.resize (3);
	m_CurPoints = 3;
	m_AutoNb = true;
}

gcpChainTool::~gcpChainTool()
{
	gnome_canvas_points_free (m_Points);
}
	
bool gcpChainTool::OnClicked()
{
	if (Element::GetMaxBonds (m_pApp->GetCurZ()) < 2)
		return false;
	m_dAngle = 0.;
	unsigned nb = (m_Length > 2)? m_Length + 1: 3;
	double a1, x, y;
	gcp::Document* pDoc = m_pView->GetDoc();
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	m_BondLength = pDoc->GetBondLength ();
	if (nb != m_CurPoints) {
		m_CurPoints = nb;
		gnome_canvas_points_free (m_Points);
		m_Points = gnome_canvas_points_new (m_CurPoints);
		if (m_CurPoints > m_Atoms.size ());
			m_Atoms.resize (m_CurPoints);
	}
	m_Positive = m_nState & GDK_LOCK_MASK;
	if (m_pObject) {
		if (m_pObject->GetType () != AtomType)
			return false;
		m_Atoms[0] = dynamic_cast<gcp::Atom*> (m_pObject);
		nb = ((gcp::Atom*) m_pObject)->GetBondsNumber ();
		m_Atoms[0]->GetCoords(&m_x0, &m_y0, NULL);
		x = m_x0 *= m_dZoomFactor;
		y = m_y0 *= m_dZoomFactor;
		m_Points->coords[0] = m_x0;
		m_Points->coords[1] = m_y0;
		switch (nb) {
		case 1: {
				map<Atom*, Bond*>::iterator i;
				gcp::Bond* bond = (gcp::Bond*) ((Atom*) m_pObject)->GetFirstBond (i);
				m_dAngle = bond->GetAngle2D ((gcp::Atom*) m_pObject);
				m_dAngle += (m_Positive)? +150: -150;
				break;
			}
		case 2: {
				double a2;
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
				m_dAngle += (m_Positive)?
						90. - pDoc->GetBondAngle () / 2.:
						pDoc->GetBondAngle () / 2. - 90;
				break;
			}
		default:
			break;
		}
	} else {
		m_Atoms[0] = NULL;
		x = m_Points->coords[0] = m_x0;
		y = m_Points->coords[1] = m_y0;
	}
	FindAtoms ();
	if (!(m_Allowed = CheckIfAllowed ()))
		return true; // true, since dragging the mouse might make things OK.
	char tmp[32];
	snprintf(tmp, sizeof(tmp) - 1, _("Bonds: %d, Orientation: %g"), m_CurPoints - 1, m_dAngle);
	m_pApp->SetStatusText(tmp);
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_line_get_type (),
								"points", m_Points,
								"fill_color", gcp::AddColor,
								"width_units", pTheme->GetBondWidth (),
								NULL);
	m_dMeanLength = pDoc->GetBondLength () * sin (pDoc->GetBondAngle () / 360. * M_PI) * m_dZoomFactor;
	m_Allowed = true; // FIXME: if MergeAtoms is true, ensure that atoms accept necessary bonds
	return true;
}

void gcpChainTool::OnDrag ()
{
	double x1, y1, x2, y2;
	unsigned nb;
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	if (m_pItem) {
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
		gtk_object_destroy (GTK_OBJECT(GNOME_CANVAS_ITEM (m_pItem)));
		gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
		m_pItem = NULL;
	}
	m_BondLength = pDoc->GetBondLength ();
	GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), m_x, m_y);
	if (pItem == (GnomeCanvasItem*) m_pBackground)
		pItem = NULL;
	Object* pObject = NULL;
	if (pItem)
		pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
	double dAngle;
	gcp::Atom *pAtom = NULL;
	if (pObject) {
		if (pObject->GetType () == BondType)
			pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x / m_dZoomFactor, m_y / m_dZoomFactor);
		else if (pObject->GetType () == FragmentType)
			pAtom = (gcp::Atom*) pObject->GetAtomAt (m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
		else if (pObject->GetType () == AtomType)
			pAtom = (gcp::Atom*) pObject;
	}
	if (pAtom && gcp::MergeAtoms) {
		// in that case, end the chain there with the current number of bonds
		pAtom->GetCoords (&m_x, &m_y, NULL);
		m_x *= m_dZoomFactor;
		m_y *= m_dZoomFactor;
		m_x-= m_x0;
		m_y -= m_y0;
		x2 = sqrt (m_x * m_x + m_y * m_y);
		if (m_CurPoints % 2 == 0) {
			x1 = m_dMeanLength * (m_CurPoints - 1);
			y1 = pDoc->GetBondLength () * cos (pDoc->GetBondAngle () / 360. * M_PI) * m_dZoomFactor;
			m_dAngle = (atan2 (-m_y, m_x) - atan2((m_Positive)? -y1: y1, x1)) / M_PI * 180.;
			m_BondLength = pDoc->GetBondLength () * x2 / x1;
		} else {
			m_dAngle = atan2 (-m_y, m_x) / M_PI * 180.;
			m_BondLength = x2 / (m_CurPoints - 1) / sin (pDoc->GetBondAngle () / 360. * M_PI) / m_dZoomFactor; 
		}
	} else {
		m_x-= m_x0;
		m_y -= m_y0;
		if (m_x == 0) {
			if (m_y == 0)
				return;
			dAngle = (m_y < 0)? 90: 270;
		} else {
		// calculate the angle and the real distance
			dAngle = atan (-m_y/m_x) * 180 / M_PI;
			if (!(m_nState & GDK_CONTROL_MASK))
				dAngle = rint(dAngle / 5) * 5;
			if (isnan (dAngle))
				dAngle = m_dAngle;
			else if (m_x < 0.)
				dAngle += 180.;
		}
		m_dAngle = dAngle;
		// Calculate number of bonds if Shift key is not pressed and we do use
		// an automatic bonds number (otherwise, change the bonds lengths,
		//	but not their number
		dAngle = atan2 (-m_y, m_x) - m_dAngle * M_PI / 180.;
		x2 = sqrt ((m_x * m_x + m_y * m_y) * cos (dAngle));
		if (m_nState & GDK_SHIFT_MASK)
			m_BondLength = x2 / (m_CurPoints - 1) / sin (pDoc->GetBondAngle () / 360. * M_PI) / m_dZoomFactor; 
		else if (m_Length < 2) {
			nb = 1 + (unsigned) rint (x2 / m_dMeanLength);
			if (nb < 3)
				nb = 3;
			if (nb != m_CurPoints) {
				m_CurPoints = nb;
				gnome_canvas_points_free (m_Points);
				m_Points = gnome_canvas_points_new (m_CurPoints);
				if (m_CurPoints > m_Atoms.size ());
					m_Atoms.resize (m_CurPoints);
			}
		}
	}
	m_Positive = m_nState & GDK_LOCK_MASK;
	m_Points->coords[0] = m_x0;
	m_Points->coords[1] = m_y0;
	FindAtoms ();
	if (!(m_Allowed = CheckIfAllowed ()))
		return;
	char tmp[32];
	snprintf (tmp, sizeof (tmp) - 1, _("Bonds: %d, Orientation: %g"), m_CurPoints - 1, m_dAngle);
	m_pApp->SetStatusText(tmp);
	m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_line_get_type (),
								"points", m_Points,
								"fill_color", gcp::AddColor,
								"width_units", pTheme->GetBondWidth (),
								NULL);
}

void gcpChainTool::OnRelease ()
{
	double x1, y1, x2, y2;
	gcp::Document* pDoc = m_pView->GetDoc ();
	unsigned nb;
	gcp::Operation *pOp = NULL;
	Object *pObject;
	char const *Id;
	gcp::Molecule *pMol = NULL;
	gcp::Bond* pBond = NULL;
	if (m_pItem) {
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
		gtk_object_destroy (GTK_OBJECT(GNOME_CANVAS_ITEM (m_pItem)));
		gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
		m_pItem = NULL;
	}
	m_pApp->ClearStatus ();
	if (!m_Allowed)
		return;
	for (nb = 0; nb < m_CurPoints; nb++) {
		if (m_Atoms[nb]) {
			if (pMol == NULL) {
				pMol = dynamic_cast<gcp::Molecule *> (m_Atoms[0]->GetMolecule ());
				pMol->Lock (true);
			}
			pOp = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			pObject = m_Atoms[0]->GetGroup ();
			Id = pObject->GetId ();
			pOp->AddObject (pObject);
			ModifiedObjects.insert (Id);
		} else {
			m_Atoms[nb] = new gcp::Atom (m_pApp->GetCurZ(),
				m_Points->coords[2 * nb] / m_dZoomFactor,
				m_Points->coords[2 * nb + 1] / m_dZoomFactor,
				0);
			pDoc->AddAtom (m_Atoms[nb]);
		}
		if (nb > 0) {
			pBond = reinterpret_cast<gcp::Bond*> (m_Atoms[nb]->GetBond (m_Atoms[nb - 1]));
			if (!pBond) {
				pBond = new gcp::Bond (m_Atoms[nb - 1], m_Atoms[nb], 1);
				pDoc->AddBond (pBond);
			}
		}
	}
	pObject = pBond->GetGroup ();
	if (pOp) {
		ModifiedObjects.insert (pObject->GetId ());
		set<string>::iterator it, end = ModifiedObjects.end ();
		for (it = ModifiedObjects.begin (); it != end; it++) {
			pObject = pDoc->GetDescendant ((*it).c_str ());
			if (pObject)
				pOp->AddObject (pObject, 1);
		}
	} else {
		pOp = pDoc->GetNewOperation (gcp::GCP_ADD_OPERATION);
		pOp->AddObject (pObject);
	}
	pDoc->FinishOperation ();
	if (pMol) {
		pMol->Lock (false);
		pMol->EmitSignal (gcp::OnChangedSignal);
	}
	ModifiedObjects.clear ();
}

static void on_length_changed (GtkSpinButton *btn, gcpChainTool *tool)
{
	tool->SetLength (gtk_spin_button_get_value (btn));
}

static void on_angle_changed (GtkSpinButton *btn, gcpChainTool *tool)
{
	tool->SetAngle (gtk_spin_button_get_value (btn));
}

static void on_merge_toggled (GtkToggleButton *btn)
{
	gcp::MergeAtoms = gtk_toggle_button_get_active (btn);
}

static void on_number_toggled (GtkToggleButton *btn, gcpChainTool *tool)
{
	bool active = gtk_toggle_button_get_active (btn);
	if (active)
		tool->SetChainLength (0);
	tool->SetAutoNumber (!gtk_toggle_button_get_active (btn));
}

static void on_number_changed (GtkSpinButton *btn, gcpChainTool *tool)
{
	tool->SetChainLength (gtk_spin_button_get_value_as_int (btn));
}

void gcpChainTool::SetAngle (double angle)
{
	m_pView->GetDoc ()->SetBondAngle (angle);
}

void gcpChainTool::SetLength (double length)
{
	m_pApp->GetActiveDocument ()->SetBondLength (length);
}

GtkWidget *gcpChainTool::GetPropertyPage ()
{
	GladeXML *xml = glade_xml_new (GLADEDIR"/chain.glade", "chain", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bond-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	m_AngleBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bond-angle"));
	g_signal_connect (m_AngleBtn, "value-changed", G_CALLBACK (on_angle_changed), this);
	m_MergeBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "merge"));
	g_signal_connect (m_MergeBtn, "toggled", G_CALLBACK (on_merge_toggled), NULL);
	m_NumberBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bonds-number"));
	gtk_widget_set_sensitive (GTK_WIDGET (m_NumberBtn), false);
	g_signal_connect (m_NumberBtn, "value-changed", G_CALLBACK (on_number_changed), this);
	GtkWidget *w = glade_xml_get_widget (xml, "auto-number");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), true);
	g_signal_connect (w, "toggled", G_CALLBACK (on_number_toggled), this);
	return glade_xml_get_widget (xml, "chain");
}

void gcpChainTool::Activate ()
{
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	gtk_spin_button_set_value (m_LengthBtn, pDoc->GetBondLength ());
	gtk_spin_button_set_value (m_AngleBtn, pDoc->GetBondAngle ());
	gtk_toggle_button_set_active (m_MergeBtn, gcp::MergeAtoms);
}

void gcpChainTool::FindAtoms ()
{
	double x1 = m_Points->coords[0], y1 = m_Points->coords[1], a;
	unsigned nb;
	for (nb = 1; nb < m_CurPoints; nb++) {
		a = (m_dAngle +
			((m_Positive ^ (nb % 2))?
				90. - m_pView->GetDoc ()->GetBondAngle () / 2.:
				m_pView->GetDoc ()->GetBondAngle () / 2. - 90))
			* M_PI / 180.;
		x1 += m_BondLength * m_dZoomFactor * cos (a); 
		y1 -= m_BondLength * m_dZoomFactor * sin (a);
		m_Atoms[nb] = NULL;
		if (gcp::MergeAtoms) {
			GnomeCanvasItem* pItem = gnome_canvas_get_item_at (GNOME_CANVAS (m_pWidget), x1, y1);
			if (pItem == (GnomeCanvasItem*) m_pBackground)
				pItem = NULL;
			Object* pObject = NULL;
			if (pItem)
				pObject = (Object*) g_object_get_data (G_OBJECT (pItem), "object");
			if (pObject && pObject != m_pObject) {
				if ((pObject->GetType () == BondType) || (pObject->GetType () == FragmentType)) {
					m_Atoms[nb] = (gcp::Atom*) pObject->GetAtomAt (x1 / m_dZoomFactor, y1 / m_dZoomFactor);
				} else if (pObject->GetType () == AtomType) {
					m_Atoms[nb] = (gcp::Atom*) pObject;
				}
			}
			if (m_Atoms[nb]) {
				m_Atoms[nb]->GetCoords(&x1, &y1, NULL);
				x1 *= m_dZoomFactor;
				y1 *= m_dZoomFactor;
			}
		}
		m_Points->coords[nb * 2] = x1;
		m_Points->coords[nb * 2 + 1] = y1;
	}
}

bool gcpChainTool::CheckIfAllowed ()
{
	unsigned i, n;
	for (i = 1; i < m_CurPoints; i++) {
		if (m_Atoms[i] == NULL)
			continue;
		n = (!m_Atoms[i]->GetBond(m_Atoms[i - 1]))? 1: 0;
		if ((i < m_CurPoints - 1) && !m_Atoms[i]->GetBond(m_Atoms[i + 1]))
			n++;
		if (n && !m_Atoms[i]->AcceptNewBonds (n))
			return false;
	}
	return true;
}
