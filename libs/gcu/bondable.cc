// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/bondable.cc
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bondable.h"

namespace gcu {

Bondable::Bondable (TypeId type): Object (type)
{
}

Bondable::~Bondable ()
{
}

unsigned Bondable::GetMaxBonds ()
{
	return 1;
}

unsigned Bondable::GetMaxBondNumber ()
{
	return 1;
}

unsigned Bondable::GetMaxMultiplicity ()
{
	return GetMaxBonds ();
}

Bond* Bondable::GetFirstBond (std::map < Bondable*, Bond* >::iterator& i)
{
	i = m_Bonds.begin ();
	if (i == m_Bonds.end ())
		return NULL;
	return (*i).second;
}

Bond const * Bondable::GetFirstBond (std::map < Bondable *, Bond * >::const_iterator& i) const
{
	i = m_Bonds.begin ();
	if (i == m_Bonds.end ())
		return NULL;
	return (*i).second;
}


Bond const * Bondable::GetNextBond (std::map < Bondable *, Bond * >::const_iterator& i) const
{
	i++;
	if (i == m_Bonds.end())
		return NULL;
	return (*i).second;
}

Bond* Bondable::GetNextBond (std::map < Bondable*, Bond * >::iterator& i)
{
	i++;
	if (i == m_Bonds.end())
		return NULL;
	return (*i).second;
}

}   //  namespace gcu
