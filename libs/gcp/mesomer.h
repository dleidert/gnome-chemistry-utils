// -*- C++ -*-

/*
 * GChemPaint library
 * mesomer.h
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

#ifndef GCHEMPAINT_MESOMER_H
#define GCHEMPAINT_MESOMER_H

#include "mechanism-step.h"
#include <map>

/*!\file*/
namespace gcp {

extern gcu::TypeId MesomerType;

class Mesomery;
class MesomeryArrow;
class Molecule;

/*!\class Mesomer gcp/mesomer.h
Represents one esomeric form in a mesomery relationship.
*/
class Mesomer: public MechanismStep
{
public:
/*!
The default constructor.
*/
	Mesomer ();
/*!
The destructor.
*/
	virtual ~Mesomer ();

/*!
@param mesomery the parent Mesomery.
@param molecule the molecule of the mesomeric form.

Constructs a Mesomer from its parent Mesomery and a molecule. If one of them is
invalid, it throws an std::invalid_argument exception and should be destroyed
since it is invalid.
*/
	Mesomer (Mesomery *mesomery, Molecule *molecule) throw (std::invalid_argument);

/*!
@param mesomery the parent Mesomery.
@param step a mesomeric form with curved arrows.

Constructs a Mesomer from its parent Mesomery and a molecule with at least one
curved arrow. If one of them is invalid, it throws an std::invalid_argument
exception and should be destroyed since it is invalid.
*/
	Mesomer (Mesomery *mesomery, MechanismStep *step) throw (std::invalid_argument);
/*!
@param node a pointer to the xmlNode containing the serialized object.

Used to load a mesomer in memory. The mesomer must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework when a signal has been emitted for the mesomer.
It should not be called by a program; call Object::EmitSignal instead.

@return true if the signal should be propagated to the parent, false otherwise.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
Used to retrieve the y coordinate for alignment. Calls gcp::Molecule::GetYAlign
for the embedded molecule and returns the result.
@return y coordinate used for alignment.
*/
	double GetYAlign ();
/*!
@param arrow a mesomery arrow
@param mesomer the mesomer at the other end of the arrow.

Adds the arrow to the arrows map. See Mesomer::GetArrows().
*/
	void AddArrow (MesomeryArrow *arrow, Mesomer *mesomer) throw (std::invalid_argument);
/*!
@param arrow a mesomery arrow
@param mesomer the mesomer at the other end of the arrow.

Removes the arrow from the arrows map. See Mesomer::GetArrows().
*/
	void RemoveArrow (MesomeryArrow *arrow, Mesomer *mesomer);
/*!
@return true if the mesomer is associated with at least one mesomery arrow, false
otherwise. See gp::Mesomery::Validtae() for more information.
*/
	bool Validate () {return m_Arrows.size () > 0;}
/*!
@return the map of all arrows pointing to this mesomer indexed by the
mesomer at the other end of the arrow.
*/
	std::map<Mesomer *, MesomeryArrow *> *GetArrows () {return &m_Arrows;}
/*!
@return th molecule associated with this mesomer.
*/
	Molecule *GetMolecule () {return m_Molecule;}
/*!
@param property the identity of the property as defined in objprops.h.

Used by the gcu::Loader mechanism to retrieve properties of mesomers.
@return the value of the property as a string.
*/
	std::string GetProperty (unsigned property) const;
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set common properties to mesomers. Only one property is
currently supported: GCU_PROP_MESOMER.
@return true if the property could be set, or if the property is not relevant,
false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
@return the localized object generic name.
*/
	std::string Name ();

private:
	Molecule *m_Molecule;
	std::map<Mesomer *, MesomeryArrow *> m_Arrows;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_MESOMER_H
