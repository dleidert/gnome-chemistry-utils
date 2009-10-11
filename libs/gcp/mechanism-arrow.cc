// -*- C++ -*-

/* 
 * GChemPaint library
 * mechanism-arrow.cc 
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mechanism-arrow.h"
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/bezier-arrow.h>
#include <gccv/canvas.h>

namespace gcp {
	
gcu::TypeId MechanismArrowType;

MechanismArrow::MechanismArrow ():
	Object (MechanismArrowType),
	gccv::ItemClient (),
	m_Source (NULL),
	m_SourceAux (NULL),
	m_Target (NULL),
	m_ShowControls (false),
	m_Pair (true)
{
}

MechanismArrow::~MechanismArrow ()
{
}

void MechanismArrow::SetSource (gcu::Object *source)
{
	m_Source = source;
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetSourceAux (gcu::Object *aux)
{
	m_SourceAux = aux;
	static_cast <Document *> (GetDocument ())->SetDirty (this);
}

void MechanismArrow::SetTarget (gcu::Object *target)
{
	m_Target = target;
	static_cast <Document *> (GetDocument ())->SetDirty (this);
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

xmlNodePtr MechanismArrow::Save (xmlDocPtr xml) const
{
	if (!m_Source || !m_Target)
		return NULL;	// this should not occur
	xmlNodePtr node = Object::Save (xml);

	return node;
}

bool MechanismArrow::Load (xmlNodePtr node)
{
	return true;
}

void MechanismArrow::Transform2D (gcu::Matrix2D& m, double x, double y)
{
}

void MechanismArrow::AddItem ()
{
	if (!m_Source || !m_Target)
		return;
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc->GetTheme ();
	double x0, y0, x1, y1, x2, y2, x3, y3, l, a;
	gcu::TypeId id = m_Source->GetType ();
	switch (id) {
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
		x0 = (x0 + x1) / 2.;
		y0 = (y0 + y1) / 2.;
		l = hypot (m_CPx1, m_CPy1);
		x0 += m_CPx1 / l * theme->GetPadding ();
		y0 += m_CPy1 / l * theme->GetPadding ();
		x1 = x0 + m_CPx1 * theme->GetZoomFactor ();
		y1 = y0 + m_CPy1 * theme->GetZoomFactor ();
		break;
	}
	case gcu::AtomType:
		break;
	default:
		break;
	}
	id = m_Target->GetType ();
	switch (id) {
	case gcu::BondType:
		break;
	case gcu::AtomType: {
		gcp::Atom *atom = static_cast <gcp::Atom *> (m_Target);
		a = atan2 (-m_CPy2, m_CPx2) * 180. / M_PI;
		atom->GetPosition (a, x3, y3);
		// convert to canvas coordinates
		x3 *= theme->GetZoomFactor ();
		y3 *= theme->GetZoomFactor ();
		// set second control point
		x2 = x3 + m_CPx2 * theme->GetZoomFactor ();
		y2 = y3 + m_CPy2 * theme->GetZoomFactor ();
		break;
	}
	default:
		break;
	}
	gccv::BezierArrow *arrow = new gccv::BezierArrow (view->GetCanvas ()->GetRoot (), this);
	arrow->SetControlPoints (x0, y0, x1, y1, x2, y2, x3, y3);
	arrow->SetShowControls (false);
	arrow->SetLineWidth (theme->GetArrowWidth ());
	arrow->SetLineColor ((view->GetData ()->IsSelected (this))? SelectColor: Color);
	// FIXME the head might be right
	arrow->SetHead (m_Pair? gccv::ArrowHeadFull: gccv::ArrowHeadLeft);
	m_Item = arrow;
}

void MechanismArrow::SetSelected (int state)
{
}

}	//	namespace gcp
