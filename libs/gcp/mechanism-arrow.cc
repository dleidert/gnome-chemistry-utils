// -*- C++ -*-

/*
 * GChemPaint library
 * mechanism-arrow.cc
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mechanism-arrow.h"
#include "mechanism-step.h"
#include <glib/gi18n.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/electron.h>
#include <gcp/fragment.h>
#include <gcp/mechanism-step.h>
#include <gcp/molecule.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/bezier-arrow.h>
#include <gccv/canvas.h>
#include <gcu/xml-utils.h>
#include <cstring>

namespace gcp {

gcu::TypeId MechanismArrowType;

MechanismArrow::MechanismArrow ():
	Object (MechanismArrowType),
	gccv::ItemClient (),
	m_Source (NULL),
	m_SourceAux (NULL),
	m_Target (NULL),
	m_ShowControls (false),
	m_Pair (true),
	m_EndAtNewBondCenter (false)
{
	SetId ("ma1");
}

MechanismArrow::~MechanismArrow ()
{
	Lock ();
	if (m_Source)
		m_Source->Unlink (this);
	if (m_SourceAux)
		m_SourceAux->Unlink (this);
	if (m_Target)
		m_Target->Unlink (this);
}

void MechanismArrow::SetSource (gcu::Object *source)
{
	if (!source)
		return;
	if (m_Source) {
		Lock ();
		m_Source->Unlink (this);
		m_Source = NULL;
		Lock (false);
	}
	m_Source = source;
	m_Source->Link (this);
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetSourceAux (gcu::Object *aux)
{
	if (!aux)
		return;
	if (m_SourceAux) {
		Lock ();
		m_SourceAux->Unlink (this);
		m_SourceAux = NULL;
		Lock (false);
	}
	m_SourceAux = aux;
	m_SourceAux->Link (this);
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetTarget (gcu::Object *target)
{
	if (!target)
		return;
	if (m_Target) {
		Lock ();
		m_Target->Unlink (this);
		m_Target = NULL;
		Lock (false);
	}
	m_Target = target;
	m_Target->Link (this);
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::OnUnlink (Object *object)
{
	if (IsLocked ())
		return;
	if (object == m_Source)
		m_Source = NULL;
	else if (object == m_SourceAux)
		m_SourceAux = NULL;
	else if (object == m_Target)
		m_Target = NULL;
	delete this;
}

void MechanismArrow::SetControlPoint (int num, double dx, double dy)
{
	switch (num) {
	case 1:
		m_CPx1 = dx;
		m_CPy1 = dy;
		break;
	case 2:
		m_CPx2 = dx;
		m_CPy2 = dy;
		break;
	default:
		// this should not occur, shouldn't we throw an exception?
		return;
	}
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetShowControls (bool show)
{
	m_ShowControls = show;
	if (m_Item)
		static_cast <gccv::BezierArrow *> (m_Item)->SetShowControls (show);
}

void MechanismArrow::SetPair (bool is_pair)
{
	m_Pair = is_pair;
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetEndAtNewBondCenter (bool end_at_new_bond_center)
{
	m_EndAtNewBondCenter = end_at_new_bond_center;
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

xmlNodePtr MechanismArrow::Save (xmlDocPtr xml) const
{
	if (!m_Source || !m_Target)
		return NULL;	// this should not occur
	xmlNodePtr node = Object::Save (xml);
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("source"), reinterpret_cast <xmlChar const *> (m_Source->GetId ()));
	if (m_SourceAux)
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("source-aux"), reinterpret_cast <xmlChar const *> (m_SourceAux->GetId ()));
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("target"), reinterpret_cast <xmlChar const *> (m_Target->GetId ()));
	xmlNewProp (node, reinterpret_cast <xmlChar const *> ("type"), reinterpret_cast <xmlChar const *> (m_Pair? "full": "single"));
	gcu::WriteFloat (node, "ct1x", m_CPx1);
	gcu::WriteFloat (node, "ct1y", m_CPy1);
	gcu::WriteFloat (node, "ct2x", m_CPx2);
	gcu::WriteFloat (node, "ct2y", m_CPy2);
	if (m_EndAtNewBondCenter)
		xmlNewProp (node, reinterpret_cast <xmlChar const *> ("end-new-bond-at-center"), reinterpret_cast <xmlChar const *> ("true"));
	return node;
}

bool MechanismArrow::Load (xmlNodePtr node)
{
	gcu::Document *doc = GetDocument ();
	if (!Object::Load (node))
	    return false;
	xmlChar *buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> ("source"));
	doc->SetTarget (reinterpret_cast <char *> (buf), &m_Source, GetParent (), this);
	if (m_Source)
		m_Source->Link (this);
	xmlFree (buf);
	buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> ("target"));
	doc->SetTarget (reinterpret_cast <char *> (buf), &m_Target, GetParent (), this);
	if (m_Target)
		m_Target->Link (this);
	xmlFree (buf);
	buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> ("source-aux"));
	if (buf) {
		doc->SetTarget (reinterpret_cast <char *> (buf), &m_SourceAux, GetParent (), this);
		if (m_SourceAux)
			m_SourceAux->Link (this);
		xmlFree (buf);
	}
	buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> ("type"));
	m_Pair = strcmp (reinterpret_cast <char *> (buf), "single");
	xmlFree (buf);
	gcu::ReadFloat (node, "ct1x", m_CPx1);
	gcu::ReadFloat (node, "ct1y", m_CPy1);
	gcu::ReadFloat (node, "ct2x", m_CPx2);
	gcu::ReadFloat (node, "ct2y", m_CPy2);
	buf = xmlGetProp (node, reinterpret_cast <xmlChar const *> ("end-new-bond-at-center"));
	if (buf) {
		m_EndAtNewBondCenter = !strcmp (reinterpret_cast <char const *> (buf), "true");
		xmlFree (buf);
	}
	doc->ObjectLoaded (this);
	return true;
}

void MechanismArrow::Transform2D (gcu::Matrix2D& m, G_GNUC_UNUSED double x, G_GNUC_UNUSED double y)
{
	m.Transform (m_CPx1, m_CPy1);
	m.Transform (m_CPx2, m_CPy2);
}

void MechanismArrow::AddItem ()
{
	if (!m_Source || !m_Target)
		return;
	if (m_Item)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	double x0, y0, x1, y1, x2, y2, x3, y3, l, a, dx = 0., dy = 0.;
	gcu::TypeId sid = m_Source->GetType (), tid = m_Target->GetType ();
	Atom *source = NULL;
	switch (sid) {
	case gcu::BondType: {
		gcp::Bond *bond = static_cast <gcp::Bond *> (m_Source);
		gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
				  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
		start->GetCoords (&x0, &y0);
		end->GetCoords (&x1, &y1);
		// convert to canvas coordinates
		x0 *= theme->GetZoomFactor ();
		y0 *= theme->GetZoomFactor ();
		x1 *= theme->GetZoomFactor ();
		y1 *= theme->GetZoomFactor ();
		if (!m_Pair) {
			dx = x1 - x0;
			dy = y1 - y0;
			l = hypot (dx, dy);
			dx = 2. * dx / l;
			dy = 2. * dy / l;
		}
		x0 = (x0 + x1) / 2.;
		y0 = (y0 + y1) / 2.;
		l = hypot (m_CPx1, m_CPy1);
		x0 += m_CPx1 / l * theme->GetPadding ();
		y0 += m_CPy1 / l * theme->GetPadding ();
		x0 /= theme->GetZoomFactor ();
		y0 /= theme->GetZoomFactor ();
		bond->AdjustPosition (x0, y0);
		x0 *= theme->GetZoomFactor ();
		y0 *= theme->GetZoomFactor ();
		x1 = x0 + m_CPx1 * theme->GetZoomFactor ();
		y1 = y0 + m_CPy1 * theme->GetZoomFactor ();
		break;
	}
	case gcu::AtomType: {
		source = static_cast <Atom *> (m_Source);
		a = atan2 (-m_CPy1, m_CPx1) * 180. / M_PI;
		source->GetPosition (a, x0, y0);
		// convert to canvas coordinates
		x0 *= theme->GetZoomFactor ();
		y0 *= theme->GetZoomFactor ();
		x1 = x0 + m_CPx1 * theme->GetZoomFactor ();
		y1 = y0 + m_CPy1 * theme->GetZoomFactor ();
		if (!m_Pair) {
			source->GetCoords (&x2, &y2);
			x2 *= theme->GetZoomFactor ();
			y2 *= theme->GetZoomFactor ();
			x2 -= x0;
			y2 -= y0;
			l = hypot (x2, y2);
			dx = 2. * y2 / l;
			dy = -2. * x2 / l;
		}
		break;
	}
	default: {
		if (m_Source->GetType () == ElectronType) {
			Object *obj = m_Source->GetParent ();
			if (obj->GetType () == gcu::FragmentType)
				source = static_cast <gcp::Fragment *> (obj)->GetAtom ();
			else
				source = static_cast <gcp::Atom *> (obj);
			double a, x, y;
			Electron *elec = static_cast <Electron *> (m_Source);
			elec->GetPosition (&a, &dx);
			a *= M_PI / 180.;
			if (dx != 0.) {
				x = dx * cos (a);
				y = -dx * sin (a);
				x *= theme->GetZoomFactor ();
				y *= theme->GetZoomFactor ();
			} else {
				source->GetRelativePosition (a * 180. / M_PI, x, y);
				x *= theme->GetZoomFactor ();
				y *= theme->GetZoomFactor ();
				x += 2. * cos (a);
				y -= 2. * sin (a);
			}
			source->GetCoords (&x0, &y0);
			x0 *= theme->GetZoomFactor ();
			y0 *= theme->GetZoomFactor ();
			x0 += x + theme->GetPadding () * cos (a);
			y0 += y - theme->GetPadding () * sin (a);
			x1 = x0 + m_CPx1 * theme->GetZoomFactor ();
			y1 = y0 + m_CPy1 * theme->GetZoomFactor ();
			if (!m_Pair && elec->IsPair ()) {
				l = hypot (x, y);
				dx = 2. * y / l;
				dy = -2. * x / l;
			}
		}
		// otherwise we should throw an exception: TODO
		break;
	}
	}
	switch (tid) {
	case gcu::BondType: {
		gcp::Bond *bond = static_cast <gcp::Bond *> (m_Target);
		gcp::Atom *start = static_cast <gcp::Atom *> (bond->GetAtom (0)),
				  *end = static_cast <gcp::Atom *> (bond->GetAtom (1));
		start->GetCoords (&x2, &y2);
		end->GetCoords (&x3, &y3);
		// convert to canvas coordinates
		x3 = (x2 + x3) / 2.;
		y3 = (y2 + y3) / 2.;
		x3 *= theme->GetZoomFactor ();
		y3 *= theme->GetZoomFactor ();
		if (!m_Pair) {
			double dx = x3 - x0, dy = y3 - y0, l = hypot (dx, dy);
			x3 -= 2. * dx / l;
			y3 -= 2. * dy / l;
		}
		l = hypot (m_CPx2, m_CPy2);
		x3 += m_CPx2 / l * theme->GetPadding ();
		y3 += m_CPy2 / l * theme->GetPadding ();
		x3 /= theme->GetZoomFactor ();
		y3 /= theme->GetZoomFactor ();
		bond->AdjustPosition (x3, y3);
		x3 *= theme->GetZoomFactor ();
		y3 *= theme->GetZoomFactor ();
		x2 = x3 + m_CPx2 * theme->GetZoomFactor ();
		y2 = y3 + m_CPy2 * theme->GetZoomFactor ();
		break;
	}
	case gcu::AtomType: {
		gcp::Atom *atom = static_cast <gcp::Atom *> (m_Target);
		if ((source != NULL || m_SourceAux != NULL) && (m_EndAtNewBondCenter || !m_Pair))
		{
			atom->GetCoords (&x3, &y3);
			x3 *= theme->GetZoomFactor ();
			y3 *= theme->GetZoomFactor ();
			if (source)
				source->GetCoords (&x2, &y2);
			else
				static_cast <gcp::Atom *> (m_SourceAux)->GetCoords (&x2, &y2);
			x2 *= theme->GetZoomFactor ();
			y2 *= theme->GetZoomFactor ();
			x3 = (x2 + x3) / 2.;
			y3 = (y2 + y3) / 2.;
			if (!m_Pair) {
				double dx = x3 - x2, dy = y3 - y2;
				l = hypot (dx, dy);
				x3 -= 2. * dx / l;
				y3 -= 2. * dy / l;
			}
		} else {
		    a = atan2 (-m_CPy2, m_CPx2) * 180. / M_PI;
			atom->GetPosition (a, x3, y3);
			// convert to canvas coordinates
			x3 *= theme->GetZoomFactor ();
			y3 *= theme->GetZoomFactor ();
			if (!m_Pair) {
				atom->GetCoords (&x2, &y2);
				x2 *= theme->GetZoomFactor ();
				y2 *= theme->GetZoomFactor ();
				x2 -= x3;
				y2 -= y3;
				l = hypot (x2, y2);
				dx = 2. * y2 / l;
				dy = -2. * x2 / l;
				if (dx * (x3 - x0) + dy * (y3 - y0) < 0) {
					x3 += dx;
					y3 += dy;
				} else {
					x3 -= dx;
					y3 -= dy;
				}
			}
		}
		// set second control point
		x2 = x3 + m_CPx2 * theme->GetZoomFactor ();
		y2 = y3 + m_CPy2 * theme->GetZoomFactor ();
		break;
	}
	default:
		break;
	}
	if (!m_Pair) {
		// This might be bad in some cases if both arrows go to the same side, but might this be chemically significant?
		if (dx * (x3 - x0) + dy * (y3 - y0) < 0) {
			x0 -= dx;
			x1 -= dx;
			y0 -= dy;
			y1 -= dy;
		} else {
			x0 += dx;
			x1 += dx;
			y0 += dy;
			y1 += dy;
		}
	}
	gccv::BezierArrow *arrow = new gccv::BezierArrow (view->GetCanvas ()->GetRoot (), this);
	arrow->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
	arrow->SetShowControls (false);
	arrow->SetLineWidth (theme->GetArrowWidth ());
	arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
	arrow->SetHead (m_Pair? gccv::ArrowHeadFull: ((x2 -x3) * (y1 - y3) - (x1 - x3) * (y2 - y3) < 0? gccv::ArrowHeadRight: gccv::ArrowHeadLeft));
	m_Item = arrow;
}

void MechanismArrow::SetSelected (int state)
{
	if (!m_Item)
		return;
	GOColor color;
	if (state != SelStateSelected)
		static_cast <gccv::BezierArrow *> (m_Item)->SetShowControls (false);
	switch (state) {
	case SelStateUnselected:
		color = Color;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = Color;
		break;
	}
	static_cast <gccv::LineItem *> (m_Item)->SetLineColor (color);
}

std::string MechanismArrow::Name ()
{
	return _("Mechanism arrow");
}

void MechanismArrow::OnLoaded ()
{
	Lock (); // don't destroy when unlinking
	if (m_Source) {
		m_Source->Unlink (this); // avoids two links
		m_Source->Link (this);
	}
	if (m_SourceAux) {
		m_SourceAux->Unlink (this); // avoids two links
		m_SourceAux->Link (this);
	}
	if (m_Target) {
		m_Target->Unlink (this); // avoids two links
		m_Target->Link (this);
	}
	Lock (false);
	Object *parent = dynamic_cast <MechanismStep *> (GetParent ());
	if (!parent) {
		// this will happen after pasting a partial selection
		parent = new MechanismStep ();
		Document *doc = static_cast <Document *> (GetDocument ());
		doc->AddChild (parent);
		parent->AddChild (this);
	}
	Molecule *mol = NULL, *mol1;
	if (m_Source) { // otherwise, it is most probably a bug
		mol = static_cast <Molecule *> (m_Source->GetMolecule ());
		if (mol && mol->GetParent () != parent && mol->GetParent ()->GetParent () != parent)
			parent->AddChild (mol);
	}
	if (m_Target && (mol1 = static_cast <Molecule *> (m_Target->GetMolecule ())) && mol1 != mol && mol1->GetParent () != parent && mol1->GetParent ()->GetParent () != parent)
		parent->AddChild (mol1);
	parent->OnLoaded ();
}

bool MechanismArrow::CanSelect () const
{
	WidgetData *data = static_cast <Document *> (GetDocument ())->GetView ()->GetData ();
	return data->IsSelected (m_Source) && data->IsSelected (m_Target) && ((m_SourceAux)? data->IsSelected (m_SourceAux): true);
}

}	//	namespace gcp
