// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * curvedarrowtool.cc 
 *
 * Copyright (C) 2004-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gtk/gtk.h>

using namespace std;

gcpCurvedArrowTool::gcpCurvedArrowTool (gcp::Application *App, string Id): gcp::Tool (App, Id)
{
	m_Full = Id == "CurvedArrow";
}

gcpCurvedArrowTool::~gcpCurvedArrowTool ()
{
}

bool gcpCurvedArrowTool::OnClicked ()
{
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l;
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gccv::ArrowHeads arrow_head = m_Full? gccv::ArrowHeadFull: gccv::ArrowHeadLeft;
	m_SourceAux = NULL;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType: {
			gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
			if (!AllowAsSource (atom))
			    return false;
			return false; // TODO: implement and return true
			break;
		}
		case gcu::BondType: {
			// try to add an arrow starting from the center of the bond and ending on the nearest atom
			gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);
			if (!AllowAsSource (bond))
			    return false;
			gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
					  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
			start->GetCoords (&x0, &y0);
			end->GetCoords (&x1, &y1);
			// convert to canvas coordinates
			x0 *= m_dZoomFactor;
			y0 *= m_dZoomFactor;
			x1 *= m_dZoomFactor;
			y1 *= m_dZoomFactor;
			x2 = m_x0 - x0;
			y2 = m_y0 - y0;
			x1 -= x0;
			y1 -= y0;			// use x3 as bond length for now
			l = hypot (x1, y1);
			double dx = x1 / l, dy = y1 / l;
			// everything beeing normalized, vector product sign will say on which side we are
			// and scalar product where we are. Let's use x3 for scalar and y3 for vector products.
			x2 /= l;
			y2 /= l;
			x3 = dx * x2 + dy * y2;
			y3 = dx * y2 - dy * x2;
			x0 += (x1 /= 2.);
			y0 += (y1 /= 2.);
			x2 = dx;
			if (y3 < 0) {
				dx = dy;
				dy = -x2;
			} else {
				dx = -dy;
				dy = x2;
			}
			if (x3 > .5) {
				m_Target = end;
				x3 = x0 + x1;
				y3 = y0 + y1;
			} else {
				m_Target = end = start;
				x3 = x0 - x1;
				y3 = y0 - y1;
			}
			// add some padding
			m_CPx0 = x0 += dx * pTheme->GetPadding ();
			m_CPy0 = y0 += dy * pTheme->GetPadding ();
			l /= 2.;
			m_CPx1 = dx * l;
			m_CPy1 = dy * l;
			x1 = x0 + m_CPx1;
			y1 = y0 + m_CPy1;
			// adjust end position
			double angle = -atan2 (dy, dx) * 180. / M_PI;
			end->GetPosition (angle, x3, y3);
			// convert to canvas coordinates
			x3 *= m_dZoomFactor;
			y3 *= m_dZoomFactor;
			// set second control point
			l += pTheme->GetArrowHeadA ();
			m_CPx2 = dx * l;
			m_CPy2 = dy * l;
			x2 = x3 + m_CPx2;
			y2 = y3 + m_CPy2;
			break;
		}
		default:
			if (m_pObject->GetType () == gcp::ElectronType) {
				if (m_Full) {
					if (!static_cast <gcp::Electron *> (m_pObject)->IsPair ())
						return false;
				}
				return false; // TODO: implement and return true
			} else if (m_pObject->GetType () == gcp::MechanismArrowType) {
				// select the arrow and show the control points
				return false; // TODO: implement and return true
			}
			break;
		}
	gccv::BezierArrow *arrow = new gccv::BezierArrow (m_pView->GetCanvas ());
	arrow->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
	arrow->SetShowControls (true);
	arrow->SetLineWidth (pTheme->GetArrowWidth ());
	arrow->SetLineColor (gcp::AddColor);
	arrow->SetHead (arrow_head);
	m_Item = arrow;
	return true;
}

void gcpCurvedArrowTool::OnDrag ()
{
	if (!m_Item || !m_pObject)
		return;	// this should not occur
	double x0 = 0., y0 = 0., x1 = 0., y1 = 0., x2 = 0., y2 = 0., x3 = 0., y3 = 0., l, dx, dy;
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	gccv::Item *item = m_pView->GetCanvas ()->GetItemAt (m_x, m_y);
	if (item == m_Item) {
		// TODO: implement moving the control points
		return;
	}
	gccv::BezierArrow *arrow = static_cast <gccv::BezierArrow *> (m_Item);
	if (item) {
		gcu::Object *cur = dynamic_cast <gcu::Object *> (item->GetClient ());
		if (cur == m_pObject) {
			switch (m_pObject->GetType ()) {
			case gcu::BondType: {
				gcp::Bond *bond = static_cast <gcp::Bond *> (m_pObject);printf("target=%p atoms: %p and %p\n",m_Target,bond->GetAtom (0),bond->GetAtom (1));
				if (!m_Target || m_Target == bond->GetAtom (0) || m_Target == bond->GetAtom (1)) {puts("got it");
					// in that case, put the arrow on the same side as the mouse pointer
					gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
							  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
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
						m_Target = (x2 < y2)? start: end;
					}
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
					x2 = dx;
					if (y3 < 0) {
						dx = dy;
						dy = -x2;
					} else {
						dx = -dy;
						dy = x2;
					}
					if (m_Target == end) {
						x3 = x0 + x1;
						y3 = y0 + y1;
					} else {
						x3 = x0 - x1;
						y3 = y0 - y1;
					}
					// add some padding
					m_CPx0 = x0 += dx * pTheme->GetPadding ();
					m_CPy0 = y0 += dy * pTheme->GetPadding ();
					l /= 2.;
					m_CPx1 = dx * l;
					m_CPy1 = dy * l;
					x1 = x0 + m_CPx1;
					y1 = y0 + m_CPy1;
					// adjust end position
					double angle = -atan2 (dy, dx) * 180. / M_PI;
					end->GetPosition (angle, x3, y3);
					// convert to canvas coordinates
					x3 *= m_dZoomFactor;
					y3 *= m_dZoomFactor;
					// set second control point
					l += pTheme->GetArrowHeadA ();
					m_CPx2 = dx * l;
					m_CPy2 = dy * l;
					x2 = x3 + m_CPx2;
					y2 = y3 + m_CPy2;
				} else {
					m_Target = NULL;
					return; // TODO: implement
				}
				break;
			}
			default:
				m_Target = NULL;
				return;	// TODO: add more types
			}
			
		} else if (cur == m_Target)
			return;
		else {
			switch (cur->GetType ()) {
			case gcu::BondType: {
				gcp::Bond *bond = static_cast <gcp::Bond *> (cur);
				if (!AllowAsTarget (bond)) {
					m_Target = NULL;
					return;
				}
				double x, y, x_, y_;
				gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
						  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
				start->GetCoords (&x, &y);
				end->GetCoords (&x_, &y_);
				m_Target = bond;
				// convert to canvas coordinates
				x *= m_dZoomFactor;
				y *= m_dZoomFactor;
				x_ *= m_dZoomFactor;
				y_ *= m_dZoomFactor;
				switch (m_pObject->GetType ()) {
				case gcu::BondType: {
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
						y_ = y0;
					}
					// start is the common atom
					x0 = m_CPx0;
					y0 = m_CPy0;
					x1 = x0 + m_CPx1;
					y1 = y0 + m_CPy1;
					x3 = (x + x_) / 2.;
					y3 = (y + y_) / 2.;
					// 
					dx = y_ -y;
					dy = x - x_;
					l = hypot (dx, dy);
					if (m_CPx1 * dy - m_CPy1 * dx > 0.) {
						dx = -dx;
						dy = -dy;
					}
					dx /= l;
					dy /= l;
					x3 += dx * pTheme->GetPadding ();
					y3 += dy * pTheme->GetPadding ();
					l += pTheme->GetArrowHeadA ();
					m_CPx2 = dx * l;
					m_CPy2 = dy * l;
					x2 = x3 + m_CPx2;
					y2 = y3 + m_CPy2;
					m_SourceAux = NULL;
					break;
				}
				case gcu::AtomType: {
					gcp::Atom *atom = static_cast <gcp::Atom *> (cur);
					if (!AllowAsTarget (atom)) {
						m_Target = NULL;
						return;
					}
				}
				default:
					m_Target = NULL;
					return;
				}
				break;
			}
			default:
				m_Target = NULL;
				return;	// TODO: add more types
			}
		}
	} else
		m_Target = NULL;
	arrow->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
}

void gcpCurvedArrowTool::OnMotion ()
{
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
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
			}
			break;
		}
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

void gcpCurvedArrowTool::OnRelease ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	else
		return;
	m_pApp->ClearStatus ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	if (!m_pObject || !m_Target)
		return;
	gcp::MechanismArrow *a = new gcp::MechanismArrow ();
	gcp::Molecule *mol = static_cast <gcp::Molecule *> (m_Target->GetMolecule ());
	// we suppose that the molecule is owned either by a mechanism step, a reaction step or the document
	// anyway, the tool must refuse all other situations
	gcu::Object *obj = mol->GetParent ();
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
	m_pView->Update (a);
	pDoc->FinishOperation ();
}

bool gcpCurvedArrowTool::AllowAsSource (gcp::Atom *atom)
{
	if (!atom->HasAvailableElectrons (m_Full))
	    return false;
	return true;
}

bool gcpCurvedArrowTool::AllowAsTarget (gcp::Atom *atom)
{
	return true;
}

bool gcpCurvedArrowTool::AllowAsSource (gcp::Bond *bond)
{
	std::set <gcu::Object *>::iterator i;
	gcu::Object* obj = bond->GetFirstLink (i);
	if (obj && obj->GetType () == gcp::MechanismArrowType) {
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
	if (obj && obj->GetType () == gcp::MechanismArrowType) {
		if (m_Full)
			return false;
		// for homolytic cleavage, the bond must be the source for both arrows
		gcp::MechanismArrow *arrow = static_cast <gcp::MechanismArrow *> (obj);
		if (arrow->GetPair () || arrow->GetTarget () != bond)
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
	default:
		return false; // TODO:manage atoms and electrons
	}
	return true;
}
