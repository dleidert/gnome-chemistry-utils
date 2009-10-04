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
#include <gcp/document.h>
#include <gccv/bezier-arrow.h>

namespace gcp {
	
gcu::TypeId MechanismArrowType;

MechanismArrow::MechanismArrow ():
	Object (MechanismArrowType),
	gccv::ItemClient (),
	m_Source (NULL),
	m_SourceAux (NULL),
	m_Target (NULL)
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
}

void MechanismArrow::SetSelected (int state)
{
}

}	//	namespace gcp
