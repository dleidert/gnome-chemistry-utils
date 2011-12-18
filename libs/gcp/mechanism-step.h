// -*- C++ -*-

/*
 * GChemPaint library
 * mechanism-step.h
 *
 * Copyright (C) 2009-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_MECHANISM_STEP_H
#define GCHEMPAINT_MECHANISM_STEP_H

/*!\file*/

#include <gcp/step.h>

namespace gcp {

extern gcu::TypeId MechanismStepType;

/*!
@brief Mechanism step.

One or several molecules with mechanism arrows joining them.
*/
class MechanismStep: public Step
{
public:
/*!
Constructs a new MechanismStep.
*/
	MechanismStep (gcu::TypeId type = MechanismStepType);
/*!
The destructor.
*/
	virtual ~MechanismStep ();

/*!
@return the alignment ordinate of the MechanismStep. Uses the average of the
values returned by the molecules.
*/
	double GetYAlign ();
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework one of the molecules or arrows has
changed. Might destroy the MechanismStep if there is not anymore an arrow.

@return true to propagate the signal to the parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param xml the xmlDoc used to save the document.

Used to save the reaction step to the xmlDoc.
@return the xmlNode containing the serialized step.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node a pointer to the xmlNode containing the serialized step.

Used to load a mechanism step in memory. The MechanismStep must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);

/*!
@return the localized object generic name.
*/
	std::string Name ();

/*!
Destroys the MechanismStep when empty.
*/
	void NotifyEmpty ();

private:
	bool m_bLoading;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_MECHANISM_STEP_H