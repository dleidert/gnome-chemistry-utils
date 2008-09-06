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

void (*Residue::m_AddCb) (Residue *res) = NULL;

Residue::Residue (): gcu::Residue ()
{
	m_Document = new Document (NULL, true, NULL);
	m_Node = m_MolNode = NULL;
	m_Refs = 0;
}

Residue::Residue (char const *name): gcu::Residue (name)
{
	m_Document = new Document (NULL, true, NULL);
	m_Node = m_MolNode = NULL;
	m_Refs = 0;
}

Residue::Residue (char const *name, char const *symbol, Molecule *mol, Document *doc): gcu::Residue (name, doc)
{
	m_Document = new Document (NULL, true, NULL);
	if (mol)
		mol->SetParent (m_Document);
	m_Molecule = mol;
	if (symbol)
		AddSymbol (symbol);
	m_Node = m_MolNode = NULL;
	if (m_AddCb && !doc && mol)
		m_AddCb (this);
	m_Refs = 0;
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
	if (m_Molecule) {
		m_Molecule->SetParent (NULL); // force destruction of children
		delete m_Molecule;
	}
	m_Molecule = new Molecule ();
	m_Document->AddChild (m_Molecule);
	static_cast <Document *> (m_Document)->SetLoading (true);
	m_Molecule->Load (m_MolNode);
	static_cast <Document *> (m_Document)->SetLoading (false);
	gcu::Residue::Load (node);
}

bool Residue::operator== (gcu::Molecule const &mol) const
{
	return *m_Molecule == mol;
}

void Residue::Register ()
{
	m_MolNode = m_Node = NULL; // forces a new node
	if (m_AddCb)
		m_AddCb (this);
}

void Residue::Ref ()
{
	m_Refs++;
	if (m_AddCb)
		m_AddCb (NULL);
}

void Residue::Unref ()
{
	if (m_Refs)
		m_Refs--;
	if (m_AddCb)
		m_AddCb (NULL);
}

}	//	namespace gcp
