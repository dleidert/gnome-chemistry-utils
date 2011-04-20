// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * curvedarrowtool.cc 
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "curvedarrowtool.h"
#include <gccv/bezier-arrow.h>
#include <gccv/canvas.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/electron.h>
#include <gcp/mechanism-arrow.h>
#include <gcp/mechanism-step.h>
#include <gcp/mesomer.h>
#include <gcp/molecule.h>
#include <gcp/reaction-step.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gcugtk/ui-builder.h>
#include <gtk/gtk.h>

using namespace std;

gcpCurvedArrowTool::gcpCurvedArrowTool (gcp::Application *App, string Id): gcp::Tool (App, Id)
{
	m_Full = Id == "CurvedArrow";
	if (m_Full) {
		GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
		m_EndAtBondCenter = go_conf_get_bool (node, "end-at-new-bond-center");
		go_conf_free_node (node);
	} else
		m_EndAtBondCenter = true;
}

gcpCurvedArrowTool::~gcpCurvedArrowTool ()
{
}

bool gcpCurvedArrowTool::OnClicked ()
{
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gccv::BezierArrow *arrow = NULL; // make g++ happy
	m_SourceAux = NULL;
	m_Target = NULL;
	m_pData->UnselectAll ();
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::FragmentType:
			m_pObject = static_cast <gcp::Fragment *> (m_pObject)->GetAtom ();
		case gcu::AtomType: {
			gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
			if (!AllowAsSource (atom))
			    return false;
			// find if there is an explicit electron pair or single electron and use it as source
			std::map <std::string, gcu::Object *>::iterator it;
			gcu::Object *obj;
			gcp::Electron *elec = NULL, *cur;
			double x, y, angle, dist, a0;
			atom->GetCoords (&x, &y);
			x *= pTheme->GetZoomFactor ();
			y *= pTheme->GetZoomFactor ();
			a0 = (x == m_x0 && y == m_y0)? go_nan: atan2 (y - m_y0, m_x0 - x);
			for (obj = atom->GetFirstChild (it); obj; obj = atom->GetNextChild (it)) {
				cur = dynamic_cast <gcp::Electron *> (obj);
				if (!cur || (m_Full && !cur->IsPair ()))
					continue;
				cur->GetPosition (&angle, &dist);
				if (elec) {
				} else {
					elec = cur;
					if (isnan (a0))
						break;
				}
			}
			if (elec) {
				elec->GetPosition (&a0, &dist);
				a0 *= M_PI / 180.;
				m_pObject = elec;
			}
			// find the most probable bond or the nearest atom
			if (atom->GetBondsNumber () > 0) {
				m_Target = atom->GetBondAtAngle (a0);
				m_Item = arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
				if (!AllowAsTarget (static_cast <gcp::Bond *> (m_Target)))
					m_Target = NULL;
				if (m_Target == NULL)
					break;
				if (m_pObject == atom)
					AtomToAdjBond ();
				else
					ElectronToAdjBond ();
			} else {
				// try to find a possible atom target
				// TODO: implement and return true
				m_Item = arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
				break;
			}
			break;
		}
		case gcu::BondType: {
			gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
			if (!AllowAsSource (bond))
			    return false;
			m_Item = arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
			BondToAdjAtom ();
			break;
		}
		default:
			if (m_pObject->GetType () == gcp::ElectronType) {
				gcp::Electron *elec = static_cast <gcp::Electron *> (m_pObject);
				if (!AllowAsSource (elec))
					return false;
				gcu::Object *obj = m_pObject->GetParent ();
				gcp::Atom *atom = static_cast <gcp::Atom *> ((obj->GetType () == gcu::AtomType)? obj: static_cast <gcp::Fragment *> (m_pObject)->GetAtom ());
				double x, a0;
				elec->GetPosition (&a0, &x);
				a0 *= M_PI / 180.;
				// find the most probable bond or the nearest atom
				if (atom->GetBondsNumber () > 0) {
					m_Target = atom->GetBondAtAngle (a0);
					m_Item = arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
					if (!AllowAsTarget (static_cast <gcp::Bond *> (m_Target)))
						m_Target = NULL;
					if (m_Target == NULL)
						break;
					ElectronToAdjBond ();
				} else {
					// try to find a possible atom target
					// TODO: implement and return true
					m_Item = arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
				}
				break;
			} else if (m_pObject->GetType () == gcp::MechanismArrowType) {
				// select the arrow and show the control points
				m_Item = arrow = static_cast <gccv::BezierArrow *> (dynamic_cast <gccv::ItemClient *> (m_pObject)->GetItem ());
				m_x0 = -1; // to make clear we have not hit a control point yet
				arrow->GetControlPoints (m_CPx0, m_CPy0, m_CPx1, m_CPy1, m_CPx2, m_CPy2, m_CPx3, m_CPy3);
				// store control point half width in m_y0
				m_y0 = arrow->GetLineWidth () * 2.5;
			}
			break;
		}
	if (arrow) {
		arrow->SetShowControls (true);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetLineColor (gcp::AddColor);
	}
	return true;
}

void gcpCurvedArrowTool::OnDrag ()
{
	if (!m_Item || !m_pObject)
		return;	// this should not occur
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gccv::BezierArrow *arrow = static_cast <gccv::BezierArrow *> (m_Item);
	if (m_pObject->GetType () == gcp::MechanismArrowType) {
			if (m_x0 < 0.) {
				if (m_x >= m_CPx1 - m_y0 && m_x <= m_CPx1 + m_y0 && m_y >= m_CPy1 - m_y0 && m_y <= m_CPy1 + m_y0) {
					// we are inside the second control point
					m_x0 = m_x;
					m_y0 = m_y;
					// store original values
					m_x1 = m_CPx1;
					m_y1 = m_CPy1;
				} else if (m_x >=m_CPx2 - m_y0 && m_x <= m_CPx2 + m_y0 && m_y >= m_CPy2 - m_y0 && m_y <= m_CPy2 + m_y0) {
					// we are inside the third control point
					m_x0 = m_x;
					m_y0 = m_y;
					m_Target = m_pObject; // to differentiate between the two control points
					// store original values
					m_x1 = m_CPx2;
					m_y1 = m_CPy2;
				}
			} else {
			// TODO: implement moving the control points
				if (m_Target) {
					m_CPx2 = m_x1 + m_x - m_x0;
					m_CPy2 = m_y1 + m_y - m_y0;
				} else {
					m_CPx1 = m_x1 + m_x - m_x0;
					m_CPy1 = m_y1 + m_y - m_y0;
				}
				arrow->SetControlPoints (m_CPx0, m_CPy0, m_CPx1, m_CPy1, m_CPx2, m_CPy2, m_CPx3, m_CPy3);				
			}
		return;
	}
	gccv::Item *item = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	if (item) {
		gcu::Object *cur = (item == m_Item)? m_Target: dynamic_cast <gcu::Object *> (item->GetClient ());
		if (!cur) { // looks that this can happen, why?
			arrow->SetControlPoints (0., 0., 0., 0., 0., 0., 0., 0.);
			return;
		}
		if (cur->GetType () == gcu::FragmentType)
			cur = static_cast <gcp::Fragment *> (cur)->GetAtom ();
		if (cur == m_pObject) {
			switch (m_pObject->GetType ()) {
			case gcu::BondType: {
				gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
				if (!m_Target || m_Target == bond->GetAtom (0) || m_Target == bond->GetAtom (1)) {
					m_Target = NULL;
					BondToAdjAtom ();
					return;
				} else {
					m_Target = NULL;
					break; // TODO: implement
				}
				break;
			}
			case gcu::AtomType: {
				gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
				if (!m_Target) {
					std::map< gcu::Atom *, gcu::Bond * >::iterator it;
					double x, y;
					atom->GetCoords (&x, &y);
					x *= pTheme->GetZoomFactor ();
					y *= pTheme->GetZoomFactor ();
					double a0 = (x == m_x && y == m_y)? go_nan: atan2 (y - m_y, m_x - x);
					if (isnan (a0))
						m_Target = atom->GetFirstBond (it);
					else
						m_Target = atom->GetBondAtAngle (a0);
					if (m_Target && AllowAsTarget (static_cast <gcp::Bond *> (m_Target)))
						AtomToAdjBond ();
					else
						m_Target = NULL;
					break;
				} else {
					gcp::Bond *bond = dynamic_cast <gcp::Bond *> (m_Target);
					if (bond && AllowAsTarget (bond))
						AtomToAdjBond ();
					else
						m_Target = NULL;
				}
				break;
			}
			default:
				if (m_pObject->GetType () == gcp::ElectronType) {
					gcp::Electron *elec = static_cast <gcp::Electron *> (m_pObject);
					gcu::Object *obj = m_pObject->GetParent ();
					gcp::Atom *atom = static_cast <gcp::Atom *> ((obj->GetType () == gcu::AtomType)? obj: static_cast <gcp::Fragment *> (m_pObject)->GetAtom ());
					double x, a0;
					elec->GetPosition (&a0, &x);
					a0 *= M_PI / 180.;
					// find the most probable bond or the nearest atom
					if (atom->GetBondsNumber () > 0) {
						m_Target = atom->GetBondAtAngle (a0);
						if (!AllowAsTarget (static_cast <gcp::Bond *> (m_Target)))
							m_Target = NULL;
						if (m_Target == NULL)
							break;
						ElectronToAdjBond ();
					}
					break;
				} else if (m_pObject->GetType () == gcp::MechanismArrowType) {
					break; // TODO: implement
				} else {
					m_Target = NULL;
					break;	// TODO: add more types
				}
			}
		} else if (cur == m_Target) {
			gcu::TypeId sid, tid;
			sid = m_pObject->GetType ();
			tid = m_Target->GetType ();
			switch (sid) {
			case gcu::BondType:
				switch (tid) {
				case gcu::BondType:
					return; // nothing should change in that case
				case gcu::AtomType: {
					gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
					if (m_Target == bond->GetAtom (0) || m_Target == bond->GetAtom (1))
						BondToAdjAtom ();
					break;
				}
				default:
					break;
				}
				break;
			case gcu::AtomType:
				break;
			default:
				break;
			}
			return;
		} else {
			switch (cur->GetType ()) {
			case gcu::BondType: {
				gcp::Bond *bond = static_cast <gcp::Bond *> (cur);
				if (!AllowAsTarget (bond)) {
					m_Target = NULL;
					break;
				}
				m_Target = bond;
				switch (m_pObject->GetType ()) {
				case gcu::BondType: {
					BondToAdjBond ();
					return;
				}
				case gcu::AtomType: {
					AtomToAdjBond ();
					return;
				}
				default:
					if (m_pObject->GetType () == gcp::ElectronType) {
						ElectronToAdjBond ();
						return;
					}
					m_Target = NULL;
					break;
				}
				break;
			}
			case gcu::AtomType: {
				gcp::Atom *atom = static_cast <gcp::Atom *> (cur);
				if (!AllowAsTarget (atom)) {
					m_Target = NULL;
					break;
				}
				m_Target = cur;
				switch (m_pObject->GetType ()) {
				case gcu::BondType: {
					gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
					if (cur == bond->GetAtom (0) || cur == bond->GetAtom (1))
						BondToAdjAtom ();
					else
						BondToAtom ();
					break;
				}
				case gcu::AtomType:
					AtomToAtom ();
					break;
				default:
					if (m_pObject->GetType () == gcp::ElectronType) {
						ElectronToAtom ();
						return;
					}
					m_Target = NULL;
				}
				break;
			}
			default:
				m_Target = NULL;
				break;	// TODO: add more types
			}
		}
	} else
		m_Target = NULL;
	if (m_Target == NULL)
		arrow->SetControlPoints (0., 0., 0., 0., 0., 0., 0., 0.);
}

void gcpCurvedArrowTool::OnMotion ()
{
	m_pData->UnselectAll ();
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::FragmentType:
			m_pObject = static_cast <gcp::Fragment *> (m_pObject)->GetAtom ();
		case gcu::AtomType:
			allowed = AllowAsSource (reinterpret_cast <gcp::Atom *> (m_pObject));
			break;
		case gcu::BondType:
			allowed = AllowAsSource (reinterpret_cast <gcp::Bond *> (m_pObject));
			break;
		default:
			if (m_pObject->GetType () == gcp::ElectronType) {
				if (m_Full)
					allowed = static_cast <gcp::Electron *> (m_pObject)->IsPair ();
				else
					allowed = true;
			} else if (m_pObject->GetType () == gcp::MechanismArrowType) {
				if (m_Full)
					allowed = static_cast <gcp::MechanismArrow *> (m_pObject)->GetPair ();
				else
					allowed = !static_cast <gcp::MechanismArrow *> (m_pObject)->GetPair ();
				if (allowed)
					static_cast <gccv::BezierArrow *> (dynamic_cast <gccv::ItemClient *> (m_pObject)->GetItem ())->SetShowControls (true);
			}
			break;
		}
	if (allowed)
		m_pData->SetSelected (m_pObject);
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

void gcpCurvedArrowTool::OnLeaveNotify ()
{
	m_pData->UnselectAll ();
}

void gcpCurvedArrowTool::OnRelease ()
{
	m_pApp->ClearStatus ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (m_Item) {
		if (m_pObject->GetType () == gcp::MechanismArrowType) {
			m_Item = NULL;
			gcp::Operation *op = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
			gcu::Object *obj = m_pObject->GetGroup ();
			op->AddObject (obj, 0);
			gcp::MechanismArrow *a = static_cast <gcp::MechanismArrow *> (m_pObject);
			if (m_Target)
				a->SetControlPoint (2, (m_CPx2 - m_CPx3) / m_dZoomFactor, (m_CPy2 - m_CPy3) / m_dZoomFactor);
			else
				a->SetControlPoint (1, (m_CPx1 - m_CPx0) / m_dZoomFactor, (m_CPy1 - m_CPy0) / m_dZoomFactor);
			m_pView->Update (m_pObject);
			op->AddObject (obj, 1);
			pDoc->FinishOperation ();
			return;
		} else {
			delete m_Item;
			m_Item = NULL;
		}
	}
	else
		return;
	if (!m_pObject || !m_Target || (m_CPx2 == 0. && m_CPy2 == 0.))
		return;
	gcp::Operation *op = pDoc->GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	gcu::Object *obj = m_pObject->GetGroup ();
	op->AddObject (obj, 0);
	if (obj != m_Target->GetGroup ())
		op->AddObject (m_Target->GetGroup (), 0);
	gcp::MechanismArrow *a = new gcp::MechanismArrow ();
	gcp::Molecule *mol = static_cast <gcp::Molecule *> (m_Target->GetMolecule ());
	// we suppose that the molecule is owned either by a mechanism step, a reaction step or the document
	// anyway, the tool must refuse all other situations
	obj = mol->GetParent ();
	if (obj->GetType () == gcu::ReactantType)
		obj = obj->GetParent ();
	if (obj->GetType () == gcu::DocumentType) {
		gcp::Molecule *mol_ = static_cast <gcp::Molecule *> (m_pObject->GetMolecule ());
		if (mol_->GetParent () != obj) {// the source is already inside a mechanism step, NOT in a reaction step
			obj = mol_->GetParent ();
			obj->AddChild (mol);
		} else {
			gcp::MechanismStep *step = new gcp::MechanismStep ();
			pDoc->AddChild (step);
			obj = step;
			obj->AddChild (mol);
			if (mol !=mol_)
				obj->AddChild (mol_);
		}
	}
	pDoc->AddObject (a);
	obj->AddChild (a);
	a->SetSource (m_pObject);
	a->SetSourceAux (m_SourceAux);
	a->SetTarget (m_Target);
	a->SetPair (m_Full);
	a->SetControlPoint (1, m_CPx1 / m_dZoomFactor, m_CPy1 / m_dZoomFactor);
	a->SetControlPoint (2, m_CPx2 / m_dZoomFactor, m_CPy2 / m_dZoomFactor);
	if (m_SetEnd)
		a->SetEndAtNewBondCenter (m_EndAtBondCenter);
	a->EmitSignal (gcp::OnChangedSignal);
	m_pView->Update (a);

	gcu::Object *group = obj->GetGroup ();
	op->AddObject ((group)? group: obj, 1);
	pDoc->FinishOperation ();
}

static void on_default (GtkToggleButton *button)
{
	GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
	go_conf_set_bool (node, "end-at-new-bond-center", gtk_toggle_button_get_active (button));
	go_conf_free_node (node);
}

static void on_end_toggled (GtkToggleButton *button, gcpCurvedArrowTool *tool)
{
	tool->SetEndAtBondCenter (gtk_toggle_button_get_active (button));
}

GtkWidget *gcpCurvedArrowTool::GetPropertyPage ()
{
	if (!m_Full)
		return NULL;
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/curvedarrowtool.ui", GETTEXT_PACKAGE);
	GtkWidget *b = builder->GetWidget ("target-btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), m_EndAtBondCenter);
	g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (on_end_toggled), this);
	GtkWidget *w = builder->GetWidget ("default");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_default), b);
	GtkWidget *res = builder->GetRefdWidget ("curvedarrow-box");
	delete builder;
	return res;
}

bool gcpCurvedArrowTool::AllowAsSource (gcp::Atom *atom)
{
	if (!atom->HasAvailableElectrons (m_Full))
	    return false;
	std::set <gcu::Object *>::iterator i;
	gcu::Object* obj = atom->GetFirstLink (i);
	while (obj && obj->GetType () != gcp::MechanismArrowType)
		obj = atom->GetNextLink (i);
	if (obj) {
		if (m_Full)
			return false;
		// for homolytic cleavage, the bond must be the source for both arrows
		gcp::MechanismArrow *arrow = static_cast <gcp::MechanismArrow *> (obj);
		if (arrow->GetPair ())
			return false;
		// only one arrow should be there
		obj = atom->GetNextLink (i);
		if (obj && obj->GetType () == gcp::MechanismArrowType)
			return false;
	}
	return true;
}

bool gcpCurvedArrowTool::AllowAsTarget (gcp::Atom *atom)
{
	if (atom == m_pObject || atom == m_pObject->GetParent ())
		return false;
	if (m_pObject->GetType () == gcu::AtomType && static_cast <gcp::Atom *> (m_pObject)->GetBond (atom))
			return false;
	if (m_pObject->GetType () == gcp::ElectronType) {
		gcu::Object *obj = m_pObject->GetParent ();
		gcp::Atom *atom0 = static_cast <gcp::Atom *> ((obj->GetType () == gcu::AtomType)? obj: static_cast <gcp::Fragment *> (m_pObject)->GetAtom ());
		if (atom0->GetBond (atom))
			return false;
	}
	if (m_pObject->GetType () == gcu::BondType && !m_Full) {
		// check if there is any existant arrow with the same source and target
		std::set <gcu::Object *>::iterator i;
		gcu::Object* obj = atom->GetFirstLink (i);
		while (obj && obj->GetType () != gcp::MechanismArrowType)
			obj = atom->GetNextLink (i);
		if (obj && static_cast <gcp::MechanismArrow *> (obj)->GetSource () == m_pObject
		    && static_cast <gcp::MechanismArrow *> (obj)->GetTarget () == atom)
			return false;
	}
	// now check that molecules are not in incompatible groups
	gcu::Object *obj1 = m_pObject->GetMolecule (), *obj2 = atom->GetMolecule ();
	if (obj1 != obj2) {
		// otherwise, no problem
		// get molecules parents
		obj1 = obj1->GetParent ();
		obj2 = obj2->GetParent ();
		// if they belong to two different reaction step, return false
		if ((obj1->GetType () == gcp::ReactionStepType || obj2->GetType () == gcp::ReactionStepType) && obj1 != obj2)
			return false;
		// no arrows between a mesomer and anything else
		if (obj1->GetType () == gcp::MesomerType || obj2->GetType () == gcp::MesomerType)
			return false;
		// otherwise we request that the two molecules have the same parent or parents inside one another (might be a bug)
		if (obj1 != obj2 && obj1->GetParent () != obj2->GetParent () && obj1->GetParent () != obj2 && obj1 != obj2->GetParent ())
			return false;
	}
	return atom->AcceptNewBonds () || atom->GetBondsNumber (); 
}

bool gcpCurvedArrowTool::AllowAsSource (gcp::Bond *bond)
{
	std::set <gcu::Object *>::iterator i;
	gcu::Object* obj = bond->GetFirstLink (i);
	while (obj && obj->GetType () != gcp::MechanismArrowType)
		obj = bond->GetNextLink (i);
	if (obj) {
		if (m_Full)
			return false;
		// for homolytic cleavage, the bond must be the source for both arrows
		gcp::MechanismArrow *arrow = static_cast <gcp::MechanismArrow *> (obj);
		if (arrow->GetPair () || arrow->GetSource () != bond)
			return false;
		// only one arrow should be there
		obj = bond->GetNextLink (i);
		if (obj && obj->GetType () == gcp::MechanismArrowType)
			return false;
	}
	return true;
}

bool gcpCurvedArrowTool::AllowAsTarget (gcp::Bond *bond)
{
	std::set <gcu::Object *>::iterator i;
	gcu::Object* obj = bond->GetFirstLink (i);
	while (obj && obj->GetType () != gcp::MechanismArrowType)
		obj = bond->GetNextLink (i);
	if (obj) {
		if (m_Full)
			return false;
		// for homolytic cleavage, the bond must be the source for both arrows
		gcp::MechanismArrow *arrow = static_cast <gcp::MechanismArrow *> (obj);
		if (arrow->GetPair () || arrow->GetTarget () != bond || arrow->GetSource () == m_pObject)
			return false;
		// only one arrow should be there
		obj = bond->GetNextLink (i);
		if (obj && obj->GetType () == gcp::MechanismArrowType)
			return false;
	}
	// now check if the source is an adjacent bond or atom
	switch (m_pObject->GetType ()) {
	case gcu::BondType: {
		gcp::Bond *bond0 = static_cast <gcp::Bond *> (m_pObject);
		if (bond->GetAtom (bond0->GetAtom (0)) == NULL && bond->GetAtom (bond0->GetAtom (1)) == NULL)
			return false;
		break;
	}
	case gcu::AtomType: {
		gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
		if (bond->GetAtom (0) != atom && bond->GetAtom (1) != atom)
			return false;
		break;
	}
	default:
		if (m_pObject->GetType () == gcp::ElectronType) {
			gcu::Object *obj = m_pObject->GetParent ();
			gcp::Atom *atom = static_cast <gcp::Atom *> ((obj->GetType () == gcu::AtomType)? obj: static_cast <gcp::Fragment *> (m_pObject)->GetAtom ());
			if (bond->GetAtom (0) != atom && bond->GetAtom (1) != atom)
				return false;
			break;
		}
		return false;
	}
	return true;
}

bool gcpCurvedArrowTool::AllowAsSource (gcp::Electron *elec)
{
	if (m_Full && !elec->IsPair ())
		return false;
	std::set <gcu::Object *>::iterator i;
	gcu::Object* obj = elec->GetFirstLink (i);
	while (obj && obj->GetType () != gcp::MechanismArrowType)
		obj = elec->GetNextLink (i);
	if (obj) {
		if (m_Full)
			return false;
		// for homolytic cleavage, the electron must be the source for both arrows
		gcp::MechanismArrow *arrow = static_cast <gcp::MechanismArrow *> (obj);
		if (arrow->GetPair ())
			return false;
		// only one arrow should be there
		obj = elec->GetNextLink (i);
		if (obj && obj->GetType () == gcp::MechanismArrowType)
			return false;
	}
	return true;
}

void gcpCurvedArrowTool::AtomToAdjBond ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l, dx, dy, s = 1.;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gcp::Bond *bond = static_cast <gcp::Bond *> (m_Target);
	gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
			  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
	if (end == m_pObject) {
		end = start;
		start = static_cast <gcp::Atom *> (m_pObject);
	}
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x1, &y1);
	// convert to canvas coordinates
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x1 *= m_dZoomFactor;
	y1 *= m_dZoomFactor;
	dx = y1 - y0;
	dy = x0 - x1;
	l = hypot (dx, dy);
	dx /= l;
	dy /= l;
	// try to find on which side we are
	if ((m_x - x0) * dx + (m_y - y0) * dy < 0) {
		dx = -dx;
		dy = -dy;
		s = -1;
	}
	x3 = ((x0 + x1) / 2 + dx * pTheme->GetPadding ());
	y3 = ((y0 + y1) / 2 + dy * pTheme->GetPadding ());
	x3 /= m_dZoomFactor;
	y3 /= m_dZoomFactor;
	bond->AdjustPosition (x3, y3);
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	l /= 2.;
	m_CPx1 = m_CPx2 = l * dx;
	m_CPy1 = m_CPy2 = l * dy;
	double a = atan2 (-m_CPy1, m_CPx1) * 180. / M_PI;
	if (start->GetPosition (a, x0, y0)) {
		// convert to canvas coordinates
		x0 *= m_dZoomFactor;
		y0 *= m_dZoomFactor;
		if (!m_Full) {
			x0 -= 2. * dy * s;
			y0 += 2. * dx * s;
			x3 += 2. * dy * s;
			y3 -= 2. * dx * s;
		}
		x1 = x0 + m_CPx1;
		y1 = y0 + m_CPy1;
		x2 = x3 + m_CPx1;
		y2 = y3 + m_CPy1;
		m_CPx0 = x0;
		m_CPy0 = y0;
		static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	} else
		x0 = y0 = m_CPx1 = m_CPx2 = m_CPy1 = m_CPy2 = x3 = y3 = 0;
	m_SetEnd = false;
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::AtomToAtom ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l, dx, dy;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gcp::Atom *start = static_cast <gcp::Atom *> (m_pObject),
			  *end = static_cast <gcp::Atom *> (m_Target);
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x3, &y3);
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	dx = x3 - x0;
	dy = y3 - y0;
	l = hypot (dx, dy);
	dx /= l;
	dy /= l;
	l = pTheme->GetBondLength () * m_dZoomFactor;
	if (start->GetBondsNumber () == 0) {
		// Set values to m_CPx1 and m_CPy1 according to the cursor position
		if ((m_x - x0) * (y3 - y0) - (m_y - y0) * (x3 - x0) < 0)
		{
			m_CPx1 = -l * dy;
			m_CPy1 = l * dx;
		} else {
			m_CPx1 = l * dy;
			m_CPy1 = -l * dx;
		}
	}
	double angle = -atan2 (m_CPy1, m_CPx1) * 180. / M_PI;
	if (start->GetPosition (angle, x0, y0)) {
		// convert to canvas coordinates
		m_CPx0 = x0 *= m_dZoomFactor;
		m_CPy0 = y0 *= m_dZoomFactor;
		x1 = x0 + m_CPx1;
		y1 = y0 + m_CPy1;
		if (!m_Full || m_EndAtBondCenter) {
			x3 = (x3 + x0) / 2.;
			y3 = (y3 + y0) / 2.;
			if (!m_Full) {
				x3 -= 2. * dx;
				y3 -= 2. * dy;
			}
			if (m_CPx1 * dy - m_CPy1 * dx < 0.) {
				m_CPx2 = -dy * l;
				m_CPy2 = dx * l;
			} else {
				m_CPx2 = dy * l;
				m_CPy2 = -dx * l;
			}
		} else {
			angle = -atan2 (m_CPy2, m_CPx2) * 180. / M_PI;
			if (end->GetPosition (angle, x3, y3)) {
				x3 *= m_dZoomFactor;
				y3 *= m_dZoomFactor;
				m_CPx2 = -dx * l;
				m_CPy2 = -dy * l;
			} else
				goto ata_err;
		}
		x2 = x3 + m_CPx2;
		y2 = y3 + m_CPy2;
		static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	} else
ata_err:
		x0 = y0 = m_CPx1 = m_CPx2= m_CPy0 = m_CPy1 = x3 = y3 = 0;
	m_SetEnd = m_Full;
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::BondToAdjAtom ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l, dx, dy, s = 1.;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
	gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
			  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
	if (start == m_Target) {
		start = end;
		end = static_cast <gcp::Atom *> (m_Target);
	} else if (m_Target && end != m_Target)
		return; // should not occur
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x1, &y1);
	// convert to canvas coordinates
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x1 *= m_dZoomFactor;
	y1 *= m_dZoomFactor;
	if (!m_Target) {
		// use the atom nearest to the mouse pointer
		x2 = hypot (x0 - m_x, y0 - m_y);
		y2 = hypot (x1 - m_x, y1 - m_y);
		if (x2 < y2) {
			m_Target = start;
			x2 = x0;
			x0 = x1;
			x1 = x2;
			y2 = y0;
			y0 = y1;
			y1 = y2;
		} else
			m_Target = end;
	}
	if (AllowAsTarget (static_cast <gcp::Atom *> (m_Target))) {
		x2 = m_x - x0;
		y2 = m_y - y0;
		x1 -= x0;
		y1 -= y0;			// use x3 as bond length for now
		l = hypot (x1, y1);
		dx = x1 / l;
		dy = y1 / l;
		// everything being normalized, vector product sign will say on which side we are
		// and scalar product where we are. Let's use x3 for scalar and y3 for vector products.
		x2 /= l;
		y2 /= l;
		x3 = dx * x2 + dy * y2;
		y3 = dx * y2 - dy * x2;
		x0 += (x1 /= 2.);
		y0 += (y1 /= 2.);
		if (!m_Full) {
			x0 += 2. * dx;
			y0 += 2. * dy;
		}
		x2 = dx;
		if (y3 < 0) {
			dx = dy;
			dy = -x2;
		} else {
			dx = -dy;
			dy = x2;
			s = -1.;
		}
		// add some padding
		x0 += dx * pTheme->GetPadding ();
		y0 += dy * pTheme->GetPadding ();
		x0 /= m_dZoomFactor;
		y0 /= m_dZoomFactor;
		bond->AdjustPosition (x0, y0);
		m_CPx0 = x0 *= m_dZoomFactor;
		m_CPy0 = y0 *= m_dZoomFactor;
		l /= 2.;
		m_CPx1 = dx * l;
		m_CPy1 = dy * l;
		x1 = x0 + m_CPx1;
		y1 = y0 + m_CPy1;
		// adjust end position
		double angle = -atan2 (dy, dx) * 180. / M_PI;
		if (static_cast <gcp::Atom *> (m_Target)->GetPosition (angle, x3, y3)) {
			// convert to canvas coordinates
			x3 *= m_dZoomFactor;
			y3 *= m_dZoomFactor;
			// set second control point
			l += pTheme->GetArrowHeadA ();
			m_CPx2 = dx * l;
			m_CPy2 = dy * l;
			if (!m_Full) {
				x3 += 2. * dy * s;
				y3 -= 2. * dx * s;
			}
			x2 = x3 + m_CPx2;
			y2 = y3 + m_CPy2;
			m_LastTarget = m_Target;
			static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
		} else
			x0 = y0 = x1 = y1 = x2 = y2 = x3 = y3 = m_CPx2 = m_CPy2 = 0;
	} else
		m_Target = NULL;
	m_SetEnd = false;
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::BondToAdjBond ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l, dx, dy;
				double x, y, x_, y_;
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	gcp::Bond *bond = static_cast <gcp::Bond *> (m_Target);
	gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
			  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
	start->GetCoords (&x, &y);
	end->GetCoords (&x_, &y_);
	// convert to canvas coordinates
	x *= m_dZoomFactor;
	y *= m_dZoomFactor;
	x_ *= m_dZoomFactor;
	y_ *= m_dZoomFactor;
	if (static_cast <gcp::Bond *> (m_pObject)->GetAtom (start) == NULL) {
		gcp::Atom *buf = start;
		start = end;
		end = buf;
		// also exchange coordinates
		x0 = x;
		x = x_;
		x_ = x0;
		x0 = y;
		y = y_;
		y_ = x0;
	}
	// start is the common atom, x and y its coordinates
	x0 = m_CPx0;
	y0 = m_CPy0;
	x1 = x0 + m_CPx1;
	y1 = y0 + m_CPy1;
	x3 = (x + x_) / 2.;
	y3 = (y + y_) / 2.;
	dx = y_ - y;
	dy = x - x_;
	l = hypot (dx, dy);
	dx /= l;
	dy /= l;
	if (!m_Full) {
		x3 += 2. * dy;
		y3 -= 2. * dx;
	}
	if ((m_CPx1 * (y0 - y) - m_CPy1 * (x0 - x)) * (dx * (y3 - y) - dy * (x3 - x)) > 0.) {
		dx = -dx;
		dy = -dy;
	}
	x3 += dx * pTheme->GetPadding ();
	y3 += dy * pTheme->GetPadding ();
	x3 /= m_dZoomFactor;
	y3 /= m_dZoomFactor;
	bond->AdjustPosition (x3, y3);
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	l /= 2.;
	l += pTheme->GetArrowHeadA ();
	m_CPx2 = dx * l;
	m_CPy2 = dy * l;
	x2 = x3 + m_CPx2;
	y2 = y3 + m_CPy2;
	m_SourceAux = NULL;
	m_SetEnd = false;
	static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::BondToAtom ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., x, y, a, dx, dy;
	gcp::Atom *start = static_cast <gcp::Atom *> (m_LastTarget),
			  *end = static_cast <gcp::Atom *> (m_Target);
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x, &y);
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x *= m_dZoomFactor;
	y *= m_dZoomFactor;
	dx = x - x0;
	dy = y - y0;
	if (!m_Full || m_EndAtBondCenter) {
		double l = hypot (dx, dy);
		dx /= l;
		dy /= l;
		x3 = (x + x0) / 2.;
		y3 = (y + y0) / 2.;
		if (!m_Full) {
			x3 -= 2. * dx;
			y3 -= 2. * dy;
		}
		if (m_CPx1 * dy - m_CPy1 * dx < 0.) {
			dx = -dx;
			dy = -dy;
		}
		x2 = x3 + (m_CPx2 = dy * pTheme->GetBondLength () * m_dZoomFactor);
		y2 = y3 + (m_CPy2 = -dx * pTheme->GetBondLength () * m_dZoomFactor);
		x0 = m_CPx0;
		y0 = m_CPy0;
		x1 = x0 + m_CPx1;
		y1 = y0 + m_CPy1;
	} else {
		a = atan2 (dy, -dx) * 180. / M_PI;
		if (end->GetPosition (a, x3, y3)) {
			x3 *= m_dZoomFactor;
			y3 *= m_dZoomFactor;
			x2 = (x0 + x) / 2.;
			y2 = (y0 + y) / 2.;
			m_CPx2 = x2 - x3;
			m_CPy2 = y2 - y3;
			x0 = m_CPx0;
			y0 = m_CPy0;
			x1 = x0 + m_CPx1;
			y1 = y0 + m_CPy1;
		} else
			x0 = y0 = x1 = y1 = m_CPx2 = m_CPy2 = 0.;
	}
	m_SourceAux = m_LastTarget;
	m_SetEnd = m_Full;
	static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::ElectronToAdjBond ()
{
	gcp::Electron *elec = static_cast <gcp::Electron *> (m_pObject);
	gcp::Bond *bond = static_cast <gcp::Bond *> (m_Target);
	gcp::Atom *atom = static_cast <gcp::Atom *> (elec->GetParent ()),
			  *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
			  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	if (end == atom) {
		end = start;
		start = atom;
	}
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., x, y, a, dx, dy, l;
	elec->GetPosition (&a, &dx);
	a *= M_PI / 180.;
	if (dx != 0.) {
		x = dx * cos (a);
		y = -dx * sin (a);
		x *= m_dZoomFactor;
		y *= m_dZoomFactor;
	} else {
		start->GetRelativePosition (a * 180. / M_PI, x, y);
		x *= m_dZoomFactor;
		y *= m_dZoomFactor;
		x += 2. * cos (a);
		y -= 2. * sin (a);
	}
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x3, &y3);
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	dx = x3 - x0;
	dy = y3 - y0;
	l = hypot (x, y);
	// store x and y in x1 and y1
	x1 = x;
	y1 = y;
	x = x / l;
	y = y / l;
	l = pTheme->GetBondLength () * m_dZoomFactor / 2.;
	m_CPx1 = x * l;
	m_CPy1 = y * l;
	l = hypot (dx, dy);
	dx /= l;
	dy /= l;
	// try to find on which side we are
	x3 = (x0 + x3) / 2.;
	y3 = (y0 + y3) / 2.;
	if (!m_Full) {
		if (elec->IsPair ()) {
			if ((y3 - y0) * x - (x3 - x0) * y < 0) {
				x0 += y * 2.;
				y0 -= x * 2.;
			} else {
				x0 -= y * 2.;
				y0 += x * 2.;
		
			}
		}
		x3 -= 2. * dx;
		y3 -= 2. * dy;
	}
	x0 += x1 + pTheme->GetPadding () * cos (a);
	y0 += y1 - pTheme->GetPadding () * sin (a);
	x1 = x0 + m_CPx1;
	y1 = y0 + m_CPy1;
	if ((m_CPy1) * dx - (m_CPx1) * dy > 0) {
		dx = -dx;
		dy = -dy;
	}
	x3 += dy * pTheme->GetPadding ();
	y3 += -dx * pTheme->GetPadding ();
	x3 /= m_dZoomFactor;
	y3 /= m_dZoomFactor;
	bond->AdjustPosition (x3, y3);
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	m_CPx2 = l * dy;
	m_CPy2 = -l * dx;
	x2 = x3 + m_CPx2;
	y2 = y3 + m_CPy2;
	static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::ElectronToAtom ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., x, y, a, dx, dy, l;
	gcp::Electron *elec = static_cast <gcp::Electron *> (m_pObject);
	gcp::Atom *start = static_cast <gcp::Atom *> (elec->GetParent ()),
			  *end = static_cast <gcp::Atom *> (m_Target);
	gcp::Theme *pTheme = m_pView->GetDoc ()->GetTheme ();
	elec->GetPosition (&a, &dx);
	a *= M_PI / 180.;
	if (dx != 0.) {
		x = dx * cos (a);
		y = -dx * sin (a);
		x *= m_dZoomFactor;
		y *= m_dZoomFactor;
	} else {
		start->GetRelativePosition (a * 180. / M_PI, x, y);
		x *= m_dZoomFactor;
		y *= m_dZoomFactor;
		x += 2. * cos (a);
		y -= 2. * sin (a);
	}
	start->GetCoords (&x0, &y0);
	end->GetCoords (&x3, &y3);
	x0 *= m_dZoomFactor;
	y0 *= m_dZoomFactor;
	x3 *= m_dZoomFactor;
	y3 *= m_dZoomFactor;
	dx = x3 - x0;
	dy = y3 - y0;
	x0 += x + pTheme->GetPadding () * cos (a);
	y0 += y - pTheme->GetPadding () * sin (a);
	l = hypot (x, y) / pTheme->GetBondLength () / m_dZoomFactor * 2.;
	x1 = x0 + (m_CPx1 = x / l);
	y1 = y0 + (m_CPy1 = y / l);
	l = hypot (dx, dy);
	dx /= l;
	dy /= l;
	// try to find on which side we are
	if ((m_CPy1) * dx - (m_CPx1) * dy > 0) {
		dx = -dx;
		dy = -dy;
	}
	if (!m_Full || m_EndAtBondCenter) {
		x3 = (x3 + x0) / 2.;
		y3 = (y3 + y0) / 2.;
		if (!m_Full) {
			x3 -= 2. * dx;
			y3 -= 2. * dy;
		}
		x2 = x3 + (m_CPx2 = dy * pTheme->GetBondLength () * m_dZoomFactor);
		y2 = y3 + (m_CPy2 = -dx * pTheme->GetBondLength () * m_dZoomFactor);
	} else {
		a = atan2 (dy, -dx) * 180. / M_PI;
		x2 = (x0 + x3) / 2.;
		y2 = (y0 + y3) / 2.;
		if (end->GetPosition (a, x3, y3)) {
			x3 *= m_dZoomFactor;
			y3 *= m_dZoomFactor;
			m_CPx2 = x2 - x3;
			m_CPy2 = y2 - y3;
		} else
			x0 = y0 = x1 = y1 = m_CPx2 = m_CPy2 = 0.;
	}
	static_cast <gccv::BezierArrow *> (m_Item)->SetHead (m_Full? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	static_cast <gccv::BezierArrow *> (m_Item)->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}
