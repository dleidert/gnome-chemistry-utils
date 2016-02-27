// -*- C++ -*-

/*
 * GChemPaint library
 * mesomer.cc
 *
 * Copyright (C) 2005-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mesomer.h"
#include "mesomery.h"
#include "molecule.h"
#include "document.h"
#include "mechanism-arrow.h"
#include "view.h"
#include <gcu/objprops.h>
#include "widgetdata.h"
#include <glib/gi18n-lib.h>

using namespace gcu;
using namespace std;

namespace gcp {

TypeId MesomerType;

Mesomer::Mesomer (): MechanismStep (MesomerType)
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
	list <MechanismArrow *> arrows;
	while (HasChildren ()) {
		Child = GetFirstChild (i);
		GetParent ()->GetParent ()->AddChild (Child);
		if (Child->GetType () == MechanismArrowType) {
			arrows.push_back (static_cast <MechanismArrow *> (Child));
			continue;
		}
		if (pOp && !Group)
			pOp->AddObject (Child, 1);
	}
	while (!arrows.empty ()) {
		MechanismArrow *arrow = arrows.front ();
		MechanismStep *step;
		Object *obj = arrow->GetSource (), *molecule = obj->GetMolecule (), *parent = molecule->GetParent ();
		if (parent->GetType () == MechanismStepType) {
			step = static_cast <MechanismStep *> (parent);
			step->AddChild (arrow);
		} else {
			step = new MechanismStep ();
			step->SetParent (parent);
			step->AddChild (arrow);
			step->AddChild (molecule);
		}
		obj = arrow->GetTarget ();
		molecule = obj->GetMolecule ();
		parent = molecule->GetParent ();
		if (parent != step) {
			if (parent->GetType () == MechanismStepType) {
				map <string, Object *>::iterator it;
				obj = parent->GetFirstChild (it);
				while (obj) {
					step->AddChild (obj);
					obj = parent->GetFirstChild (it);
				}

			} else
				step->AddChild (molecule);
		}
		arrows.pop_front ();
	}
}

Mesomer::Mesomer (Mesomery* mesomery, Molecule* molecule) throw (std::invalid_argument): MechanismStep (MesomerType)
{
	if (!mesomery || !molecule)
		throw invalid_argument ("NULL argument to Mesomer constructor!");
	SetId ("ms1");
	mesomery->AddChild (this);
	GetDocument ()->EmptyTranslationTable();
	AddChild (molecule);
	m_Molecule = molecule;
}

Mesomer::Mesomer (Mesomery *mesomery, MechanismStep *step) throw (std::invalid_argument): MechanismStep (MesomerType)
{
	if (!mesomery || !step)
		throw invalid_argument (_("NULL argument to Mesomer constructor!"));
	SetId ("ms1");
	mesomery->AddChild (this);
	GetDocument ()->EmptyTranslationTable();
	map <string, Object *>::iterator it;
	while (Object *obj = step->GetFirstChild (it)) {
		if (obj->GetType () == MoleculeType)
			m_Molecule = static_cast <Molecule *> (obj);
		AddChild (obj);
	}
}

void Mesomer::AddArrow (MesomeryArrow *arrow, Mesomer *mesomer) throw (std::invalid_argument)
{
	if (m_Arrows[mesomer])
		throw invalid_argument (_("Only one arrow can link two given mesomers."));
	m_Arrows[mesomer] = arrow;
}

void Mesomer::RemoveArrow (G_GNUC_UNUSED MesomeryArrow *arrow, Mesomer *mesomer)
{
	m_Arrows.erase (mesomer);
}

double Mesomer::GetYAlign ()
{
	return (m_Molecule)? m_Molecule->GetYAlign (): 0.;
}

bool Mesomer::OnSignal (G_GNUC_UNUSED SignalId Signal, G_GNUC_UNUSED Object *Child)
{
	if (GetChildrenNumber () != 1)
		delete this;
	return true;
}

bool Mesomer::Load (xmlNodePtr node)
{
	if (Object::Load (node)) {
		map<string, Object*>::iterator i;
		Object *obj = GetFirstChild (i);
		while (obj && obj->GetType () != MoleculeType)
			obj = GetNextChild (i);
		if (!obj)
			return false;
		m_Molecule = reinterpret_cast <Molecule *> (obj);
		GetDocument ()->ObjectLoaded (this);
		return true;
	}
	return false;
}

std::string Mesomer::GetProperty (unsigned property) const
{
	std::string res;
	switch (property) {
	case GCU_PROP_MESOMER:
		res = m_Molecule->GetId ();
		break;
	default:
		return Object::GetProperty (property);
	}
	return res;
}

bool Mesomer::SetProperty (unsigned property, char const *value)
{
	gcu::Document *doc = GetDocument ();
	switch (property) {
	case GCU_PROP_MESOMER:
		if (m_Molecule != NULL && !strcmp (m_Molecule->GetId (), value)) {
			break;
		}
		if (m_Molecule != NULL)
			m_Molecule->SetParent (doc);
		m_Molecule = dynamic_cast < gcp::Molecule * > (doc->GetDescendant (value));
		if (m_Molecule != NULL)
			AddChild (m_Molecule);
		break;
	}
	return true;
}

std::string Mesomer::Name ()
{
	return _("Mesomer");
}

}	//	namespace gcp
