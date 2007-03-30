// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomer.cc
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mesomer.h"
#include "mesomery.h"
#include "molecule.h"
#include "document.h"
#include <glib/gi18n-lib.h>

namespace gcp {

TypeId MesomerType;

Mesomer::Mesomer (): Object (MesomerType)
{
}

Mesomer::~Mesomer ()
{
	if (IsLocked ())
		return;
	Document *pDoc = reinterpret_cast<Document *> (GetDocument ());
	Operation *pOp = pDoc->GetCurrentOperation ();
	Mesomery *ms = reinterpret_cast<Mesomery *> (GetParent ());
	if (!ms)
		return;
	map<string, Object *>::iterator i;
	Object *Child, *Group = ms->GetGroup ();
	while (HasChildren ()) {
		Child = GetFirstChild (i);
		GetParent ()->GetParent ()->AddChild (Child);
		if (pOp && !Group)
			pOp->AddObject (Child, 1);
	}
}

Mesomer::Mesomer (Mesomery* mesomery, Molecule* molecule) throw (std::invalid_argument): Object (MesomerType)
{
	if (!mesomery || !molecule)
		throw invalid_argument ("NULL argument to Mesomer constructor!");
	SetId ("ms1");
	mesomery->AddChild (this);
	GetDocument ()->EmptyTranslationTable();
	AddChild (molecule);
	m_Molecule = molecule;
}

void Mesomer::AddArrow (MesomeryArrow *arrow, Mesomer *mesomer) throw (std::invalid_argument)
{
	if (m_Arrows[mesomer])
		throw invalid_argument (_("Only one arrow can link two given mesomers."));
	m_Arrows[mesomer] = arrow;
}

void Mesomer::RemoveArrow (MesomeryArrow *arrow, Mesomer *mesomer)
{
	m_Arrows.erase (mesomer);
}

double Mesomer::GetYAlign ()
{
	return (m_Molecule)? m_Molecule->GetYAlign (): 0.;
}

bool Mesomer::OnSignal (SignalId Signal, Object *Child)
{
	if (GetChildrenNumber () != 1) {
		delete GetParent ();
		return false;
	}
	return true;
}

bool Mesomer::Load (xmlNodePtr node)
{
	if (Object::Load (node)) {
		if (GetChildrenNumber () != 1)
			return false;
		map<string, Object*>::iterator i;
		m_Molecule = reinterpret_cast<Molecule *> (GetFirstChild (i));
		return true;
	}
	return false;
}

}	//	namespace gcp
