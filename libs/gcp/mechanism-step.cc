// -*- C++ -*-

/* 
 * GChemPaint library
 * mechanism-step.cc 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "mechanism-step.h"
#include "mechanism-arrow.h"
#include "molecule.h"
#include "reaction-step.h"
#include <glib/gi18n.h>

namespace gcp {
	
gcu::TypeId MechanismStepType;
extern gcu::SignalId OnChangedSignal;

MechanismStep::MechanismStep (gcu::TypeId type):
	Object (type),
	m_bLoading (false)
{
	SetId ("ms1");
}

MechanismStep::~MechanismStep ()
{
}

std::string MechanismStep::Name ()
{
	return _("Mechanism step");
}

double MechanismStep::GetYAlign ()
{
	unsigned nb = 0;
	double res = 0.;
	std::map <std::string, Object *>::iterator i;
	for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
		if (obj->GetType () == gcu::MoleculeType) {
			res += obj->GetYAlign ();
			nb++;
		}
	return res / nb;
}

bool MechanismStep::OnSignal (gcu::SignalId Signal, G_GNUC_UNUSED gcu::Object *Child)
{
	if (Signal == OnChangedSignal) {
		if (m_bLoading)
			return false;
		std::map <std::string, Object *>::iterator i;
		std::set <Object *> molecules;
		for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
			if (obj->GetType () == MechanismArrowType) {
				MechanismArrow *arrow = static_cast <MechanismArrow *> (obj);
				Object *mol = arrow->GetSource ()->GetMolecule ();
				if (mol)
					molecules.insert (mol);
				mol = arrow->GetTarget ()->GetMolecule ();
				if (mol)
					molecules.insert (mol);
			}
		if (molecules.empty ()) {
			// if no arrow remains, delete this
			Object *parent = GetParent ();
			Object *obj;
			SetParent (NULL); //
			if (parent->GetType () == ReactionStepType) {
				ReactionStep *step = static_cast <ReactionStep *> (parent);
				while ((obj = GetFirstChild (i)))
					if (obj->GetType () == gcu::MoleculeType)
						step->AddMolecule (static_cast <Molecule *> (obj), false);
			} else
				while ((obj = GetFirstChild (i)))
					parent->AddChild (obj);
			delete this;
			parent->EmitSignal (OnChangedSignal);
			return false;
		} else {
			Object *parent = GetParent ();
			Object *obj;
			ReactionStep *step = static_cast <ReactionStep *> (parent);
			std::set <Object *> orphans;
			std::set <Object *>::iterator j, end = molecules.end ();
			// search for molecules without a mechanism arrow
			for (obj = GetFirstChild (i); obj; obj = GetNextChild (i))
				if (molecules.find (obj) == end)
					orphans.insert (obj);
			// now remove orphans from this
			j = orphans.end ();
			for (j = orphans.begin (); j != end; j++) {
				if (step)
					step->AddMolecule (static_cast <Molecule *> (*j), false);
				else
					parent->AddChild (*j);
			}
		}
	}
	return true;
}

bool MechanismStep::Load (xmlNodePtr node)
{
	m_bLoading = true;
	bool res = gcu::Object::Load (node);
	m_bLoading = false;
	return res;
}

}	//	namespace gcp
