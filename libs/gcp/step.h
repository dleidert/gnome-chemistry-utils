// -*- C++ -*-

/*
 * GChemPaint library
 * step.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_STEP_H
#define GCHEMPAINT_STEP_H

#include <gcu/object.h>

/*!\file*/
namespace gcp {

class Arrow;

/*!\class Step gcp/step.h
\brief parent class for reaction steps, mesomers, and more.
*/

class Step: public gcu::Object
{
public:
/*!
The default constructor.
*/
	Step (gcu::TypeId type);
/*!
The destructor.
*/
	virtual ~Step ();

/*!
@param arrow an arrow
@param step the Step at the other end of the arrow.

Adds the arrow to the arrows map. See Step::GetArrows().
*/
	void AddArrow (Arrow *arrow, Step *step) throw (std::invalid_argument);
/*!
@param arrow an arrow
@param step the Step at the other end of the arrow.

Removes the arrow from the arrows map. See Step::GetArrows().
*/
	void RemoveArrow (Arrow *arrow, Step *step);
/*!
@return true if the step is associated with at least one arrow, false
otherwise. See gcp::Scheme::Validate() for more information.
*/
	bool Validate () {return m_Arrows.size () > 0;}
/*!
@return the map of all arrows pointing to this Step indexed by the
Step at the other end of the arrow.
*/
	std::map < Step *, Arrow * > *GetArrows () {return &m_Arrows;}

protected:
	std::map < Step *, Arrow * > m_Arrows;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_STEP_H
