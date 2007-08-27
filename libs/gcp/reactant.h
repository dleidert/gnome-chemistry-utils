// -*- C++ -*-

/* 
 * GChemPaint library
 * reactant.h 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTANT_H
#define GCHEMPAINT_REACTANT_H

#include <gcu/object.h>

using namespace gcu;

namespace gcp {

class ReactionStep;

class Reactant: public Object
{
public:
	Reactant ();
	virtual ~Reactant ();

	Reactant (ReactionStep* step, Object* object) throw (std::invalid_argument);
	
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr);
	unsigned GetStoich () {return m_Stoich;}
	void SetStoich (unsigned Stoich) {m_Stoich = Stoich;}

	unsigned GetStoichiometry () {return m_Stoich;}
	void SetStoichiometry (unsigned coef) {m_Stoich = coef;}
	virtual double GetYAlign ();
	virtual bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	
	void AddStoichiometry ();
	Object *GetChild () {return Child;}
	Object *GetStoichChild () {return Stoichiometry;}

private:
	unsigned m_Stoich;	//always positive
	Object *Stoichiometry;
	Object *Child;
};

}	//	namespace gcp

#endif //	GCHEMPAINT_REACTANT_H