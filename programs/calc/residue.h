// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/calc/residue.h 
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

#ifndef GCC_RESIDUE_H
#define GCC_RESIDUE_H

#include <gcu/residue.h>

class gccResidue: public gcu::Residue
{
public:
	gccResidue ();
	gccResidue (char const *name);
	virtual ~gccResidue ();

	std::map<int,int> const &GetRawFormula () {return m_Raw;}
	virtual void Load (xmlNodePtr node);

private:
	std::map<int,int> m_Raw;
};

#endif	//	GCC_RESIDUE_H
