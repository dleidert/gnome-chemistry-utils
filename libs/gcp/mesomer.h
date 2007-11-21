// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomer.h 
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

#ifndef GCHEMPAINT_MESOMER_H
#define GCHEMPAINT_MESOMER_H

#include <gcu/object.h>
#include <map>

namespace gcp {

extern gcu::TypeId MesomerType;

class Mesomery;
class MesomeryArrow;
class Molecule;

class Mesomer: public gcu::Object
{
public:
	Mesomer ();
	virtual ~Mesomer ();
	
	Mesomer (Mesomery *mesomery, Molecule *molecule) throw (std::invalid_argument);
	virtual bool Load (xmlNodePtr);
	virtual bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
	virtual double GetYAlign ();
	void AddArrow (MesomeryArrow *arrow, Mesomer *mesomer) throw (std::invalid_argument);
	void RemoveArrow (MesomeryArrow *arrow, Mesomer *mesomer);
	bool Validate () {return m_Arrows.size () > 0;}
	std::map<Mesomer *, MesomeryArrow *> *GetArrows () {return &m_Arrows;}
	Molecule *GetMolecule () {return m_Molecule;}

private:
	Molecule *m_Molecule;
	std::map<Mesomer *, MesomeryArrow *> m_Arrows;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_MESOMER_H
