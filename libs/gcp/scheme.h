// -*- C++ -*-

/*
 * GChemPaint library
 * step.h
 *
 * Copyright (C) 2011-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_SCHEME_H
#define GCHEMPAINT_SCHEME_H

#include <gcu/object.h>

/*!\file*/
namespace gcp {

/*!\class Scheme gcp/scheme.h
\brief parent class for reaction, mesomery, and anything else containing chemical
objects and arrows between them.
*/

class Scheme: public gcu::Object
{
public:
/*!
@param type the actual type of the object.

The default constructor.
*/
	Scheme (gcu::TypeId type);

/*!
The destructor.
*/
	virtual ~Scheme ();

protected:
/*!
Aligns the children logically.
*/
	void Align () throw (std::invalid_argument);
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_STEP_H
