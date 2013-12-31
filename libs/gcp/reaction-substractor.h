// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-substractor.h
 *
 * Copyright (C) 2013 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_SUBSTRACTOR_H
#define GCHEMPAINT_REACTION_SUBSTRACTOR_H

#include <gcp/reaction-separator.h>
#include <gcu/object.h>

/*!\file*/
namespace gcp {

extern gcu::TypeId ReactionSubstractorType;

/*!\class ReactionSubstractor gcp/reaction-substractor.h
\brief Class for "−" strings used for reaction products attached to an arrow.

Objects of this class are added when useful by the framework. There is no need
to create them manually.
*/
class ReactionSubstractor: public ReactionSeparator
{
public:
/*!
The constructor. Adds a ", " string to separate objects attached to an arrow.
*/
	ReactionSubstractor ();
/*!
The destructor.
*/
	virtual ~ReactionSubstractor ();
/*!
@return the localized object generic name.
*/
	std::string Name ();

private:
	double m_x, m_y;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_SEPARATOR_H
