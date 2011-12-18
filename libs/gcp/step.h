// -*- C++ -*-

/*
 * GChemPaint library
 * step.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
@param arrow an arrow related to this instance.

Adds an arrow to the list of arrows related to this instance.
*/
	void AddArrow (Arrow *arrow) {m_Arrows.insert (arrow);}
/*!
@param arrow an arrow related to this instance.

Removes an arrow from the list of arrows related to this instance when it
not anymore relevant.
*/
	void RemoveArrow (Arrow *arrow);

protected:
	std::set < Arrow *> m_Arrows;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_STEP_H
