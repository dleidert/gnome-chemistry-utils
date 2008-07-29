// -*- C++ -*-

/* 
 * GChemPaint library
 * residue.h 
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "molecule.h"
#include <iostream>
#include <cstring>

xmlDocPtr user_residues = NULL;

using namespace std;

namespace gcp
{

Residue::Residue (): gcu::Residue ()
{
	m_Doc = new Document (NULL, true, NULL);
}

Residue::Residue (char const *name): gcu::Residue (name)
{
	m_Doc = new Document (NULL, true, NULL);
}

Residue::Residue (char const *name, char const *symbol, Molecule *mol, Document *doc): gcu::Residue (name, doc)
{
	m_Doc = new Document (NULL, true, NULL);
	mol->SetParent (m_Doc);
	m_Molecule = mol;
	AddSymbol (symbol);
}

Residue::~Residue ()
{
	delete m_Doc;
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
	if (m_Molecule)
		delete m_Molecule;
	m_Molecule = new Molecule ();
	m_Doc->AddChild (m_Molecule);
	m_Doc->SetLoading (true);
	m_Molecule->Load (m_MolNode);
	m_Doc->SetLoading (false);
	map<string, gcu::Object*>::iterator i;
	m_Molecule = dynamic_cast <Molecule*> (m_Doc->GetFirstChild (i));
	gcu::Residue::Load (node);
}

bool Residue::operator== (gcu::Molecule const &mol) const
{
	return *m_Molecule == mol;
}

}	//	namespace gcp
