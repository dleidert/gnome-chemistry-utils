// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * atom.cc
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

#include "element.h"
#include "atom.h"
#include "bond.h"
#include "xml-utils.h"
#include <cmath>

using namespace gcu;

Atom::Atom (): Object (AtomType)
{
	m_Z = 0;
	m_x = m_y = m_z = 0.0;
	m_Charge = 0;
}

Atom::~Atom ()
{
	m_Bonds.clear ();
}

Atom::Atom (int Z, double x, double y, double z):
	Object (AtomType)
{
	SetZ (Z);
	m_x = x;
	m_y = y;
	m_z = z;
	m_Charge = 0;
}

Atom::Atom (Atom& a):
	Object (AtomType)
{
	SetZ (a.m_Z);
	m_x = a.m_x;
	m_y = a.m_y;
	m_z = a.m_z;
	m_Charge = a.m_Charge;
}

Atom& Atom::operator= (Atom& a)
{
	SetZ (a.m_Z);
	m_x = a.m_x;
	m_y = a.m_y;
	m_z = a.m_z;
	m_Charge = a.m_Charge;
	return *this ;
}

void Atom::SetZ (int Z)
{
	m_Z = Z;
}

double Atom::Distance (Atom* pAtom)
{
	return sqrt (square (m_x - pAtom->m_x) + square (m_y - pAtom->m_y) + square (m_z - pAtom->m_z));
}

void Atom::zoom (double ZoomFactor)
{
	m_x *= ZoomFactor;
	m_y *= ZoomFactor;
	m_z *= ZoomFactor;
}

bool Atom::GetCoords (double *x, double *y, double *z)
{
	if (!x || !y)
		return false;
	*x = m_x;
	*y = m_y;
	if (z)
		*z = m_z;
	return true;
}

const gchar* Atom::GetSymbol ()
{
	Element* Elt = Element::GetElement(m_Z);
	return (Elt)? Element::Symbol(m_Z): NULL;
}

void Atom::AddBond (Bond* pBond)
{
	m_Bonds[pBond->GetAtom (this)] = pBond;
}

void Atom::RemoveBond (Bond* pBond)
{
	m_Bonds.erase (pBond->GetAtom (this));
}

Bond* Atom::GetFirstBond (map<Atom*, Bond*>::iterator& i)
{
	i = m_Bonds.begin ();
	if (i == m_Bonds.end ())
		return NULL;
	return (*i).second;
}

Bond* Atom::GetNextBond (map<Atom*, Bond*>::iterator& i)
{
	i++;
	if (i == m_Bonds.end())
		return NULL;
	return (*i).second;
}

void Atom::Move (double x, double y, double z)
{
	m_x += x;
	m_y += y;
	m_z += z;
}

void Atom::Transform2D (Matrix2D& m, double x, double y)
{
	m_x -= x;
	m_y -= y;
	m.Transform (m_x, m_y);
	m_x += x;
	m_y += y;
}

xmlNodePtr Atom::Save (xmlDocPtr xml)
{
	xmlNodePtr parent;
	gchar buf[16];
	parent = xmlNewDocNode (xml, NULL, (xmlChar*) "atom", NULL);
	if (!parent)
		return NULL;
	SaveId (parent);

	if (m_Z) {
		strncpy(buf, GetSymbol(), sizeof(buf));
		xmlNewProp(parent, (xmlChar*) "element", (xmlChar*) buf);
	}
	
	if (m_Charge) {
		snprintf (buf, sizeof (buf), "%d", m_Charge);
		xmlNewProp (parent, (xmlChar*) "charge", (xmlChar*) buf);
	}
	if (!WritePosition (xml, parent, NULL, m_x, m_y, m_z)) {
		xmlFreeNode (parent);
		return NULL;
	}
	if (!SaveNode (xml, parent)) {
		xmlFreeNode (parent);
		return NULL;
	}
	return parent;
}

bool Atom::Load (xmlNodePtr node)
{
	char* tmp;
	tmp = (char*) xmlGetProp (node, (xmlChar*) "id");
	if (tmp) {
		SetId (tmp);
		xmlFree (tmp);
	}
	tmp = (char*) xmlGetProp (node, (xmlChar*) "element");
	if (tmp) {
		m_Z = Element::Z(tmp);	//Don't check if element exists. Applications that do not accept unknown elements should check
		xmlFree (tmp);
	}
	tmp = (char*) xmlGetProp (node, (xmlChar*) "charge");
	if (tmp) {
		m_Charge = (char) atoi (tmp);
		xmlFree (tmp);
	} else
		m_Charge = 0;
	if (!ReadPosition (node, NULL, &m_x, &m_y, &m_z) || (!LoadNode (node))) 
		return false;
	return true;
}

bool Atom::LoadNode (xmlNodePtr node)
{
	return true;
}

bool Atom::SaveNode (xmlDocPtr xml, xmlNodePtr node)
{
	return true;
}

Bond* Atom::GetBond (Atom* pAtom)
{
	Bond* pBond = m_Bonds[pAtom];
	if (!pBond)
		m_Bonds.erase (pAtom);
	return pBond;
}
