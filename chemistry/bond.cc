// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * bond.cc 
 *
 * Copyright (C) 2001-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "bond.h"
#include "atom.h"
//#include "cycle.h"
//#include "settings.h"
//#include "document.h"
#include <math.h>

using namespace gcu;

Bond::Bond(): Object(BondType)
{
	m_Begin = NULL;
	m_End = NULL;
	m_order = 0;
}

Bond::Bond(Atom* first, Atom* last, unsigned char order): Object(BondType)
{
	m_Begin = first;
	m_End = last;
	m_order = order;
	first->AddBond(this);
	last->AddBond(this);
}

Bond::~Bond()
{
}

Atom* Bond::GetAtom(int which)
{
	switch (which)
	{
	case 0: return m_Begin;
	case 1: return m_End;
	default: return NULL;
	}
}

Atom* Bond::GetAtom(Atom* pAtom, int which)
{
	return (pAtom == m_Begin)? m_End: (pAtom == m_End)? m_Begin: NULL;
}

unsigned char Bond::GetOrder()
{
	return m_order;
}

void Bond::SetOrder(unsigned char Order)
{
	m_order = Order;
}

xmlNodePtr Bond::Save(xmlDocPtr xml)
{
	xmlNodePtr parent, child;
	gchar buf[16];
	parent = xmlNewDocNode(xml, NULL, (xmlChar*)"bond", NULL);
	if (!parent) return NULL;

	SaveId(parent);

	buf[0] = m_order + '0';
	buf[1] = 0;
	xmlNewProp(parent, (xmlChar*)"order", (xmlChar*)buf);

	xmlNewProp(parent, (xmlChar*)"begin", (xmlChar*)m_Begin->GetId());

	xmlNewProp(parent, (xmlChar*)"end", (xmlChar*)m_End->GetId());

	if (!SaveNode(xml, parent)) {xmlFreeNode(parent); return NULL;}
	return parent;
}

bool Bond::Load(xmlNodePtr node)
{
	char* tmp;
	xmlNodePtr child;
	Object* pObject;
	tmp = (char*)xmlGetProp(node, (xmlChar*)"id");
	if (tmp)
	{
		SetId(tmp);
		xmlFree(tmp);
	}
	tmp = (char*)xmlGetProp(node, (xmlChar*)"order");
	if (!tmp) m_order = 1;
	else
	{
		m_order = *tmp - '0';
		xmlFree(tmp);
	}
	if ((m_order < 1) || (m_order > 4)) return false;
	tmp = (char*)xmlGetProp(node, (xmlChar*)"begin");
	if (!tmp)
	{
		child = GetNodeByName(node, "begin");
		tmp = (char*)xmlNodeGetContent(child); //necessary to read version 0.1.0 files
		if (!tmp) return false;
	}
	pObject = GetParent()->GetDescendant(tmp);
	xmlFree(tmp);
	if (!pObject || (pObject->GetType() != AtomType)) return false;
	m_Begin = (Atom*)(pObject);
	tmp = (char*)xmlGetProp(node, (xmlChar*)"end");
	if (!tmp)
	{
		child = GetNodeByName(node, "end");
		tmp = (char*)xmlNodeGetContent(child); //necessary to read version 0.1.0 files
		if (!tmp) return false;
	}
	pObject = GetParent()->GetDescendant(tmp);
	xmlFree(tmp);
	if (!pObject || (pObject->GetType() != AtomType)) return false;
	m_End = (Atom*)pObject;
	m_Begin->AddBond(this);
	m_End->AddBond(this);
	return LoadNode(node);
}

bool Bond::LoadNode(xmlNodePtr)
{
	return true;
}

bool Bond::SaveNode(xmlDocPtr, xmlNodePtr)
{
	return true;
}

void Bond::IncOrder(int n)
{
	m_order += n;
	if (m_order > 4)  m_order %= 4;
}
