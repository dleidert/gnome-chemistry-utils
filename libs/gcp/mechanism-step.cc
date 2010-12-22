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
		for (Object *obj = GetFirstChild (i); obj; obj = GetNextChild (i))
			if (obj->GetType () == MechanismArrowType)
				return true;
		// if no arrow remains, delete this
		delete this;
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
