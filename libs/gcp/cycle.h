// -*- C++ -*-

/* 
 * GChemPaint library
 * cycle.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_CYCLE_H
#define GCHEMPAINT_CYCLE_H
#include "chain.h"

namespace gcp {

class Cycle: public Chain
{
public:
	Cycle (Molecule* Molecule);
	virtual ~Cycle ();
	
	void Simplify ();	//Reduce size of fused cycles
	virtual void Erase (Atom* pAtom1, Atom* pAtom2);
	virtual void Insert (Atom* pAtom1, Atom* pAtom2, Chain& Chain);
	bool IsBetterForBonds (Cycle* pCycle);
	void GetAngles2D (Bond *pBond, double* a1, double* a2);
	int GetFusedBonds ();
};

}	//	namespace gcp

#endif // GCHEMPAINT_CYCLE_H
