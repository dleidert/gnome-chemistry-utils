// -*- C++ -*-

/* 
 * GChemPaint library
 * residue.h 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "residue.h"
#include <iostream>

namespace gcp
{

Residue::Residue (): gcu::Residue ()
{
}

Residue::Residue (char const *name): gcu::Residue (name)
{
}

Residue::~Residue ()
{
}

void Residue::Load (xmlNodePtr node, bool ro)
{
	m_ReadOnly = ro;
	m_Node = node;
	m_MolNode = node->children;
	while (m_MolNode && strcmp ((char const *) m_MolNode->name, "molecule"))
		   m_MolNode = m_MolNode->next;
	if (!m_MolNode) {
		cerr << "Invalid residue" << endl;
		delete this;
		return;
	}
	gcu::Residue::Load (node);
}

}	//	namespace gcp
