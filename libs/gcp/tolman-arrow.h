// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcp/tolman-arrow.h
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_TOLMAN_ARROW_H
#define GCP_TOLMAN_ARROW_H

#include <gcu/object.h>

namespace gcp {

extern gcu::TypeId TolmanArrowType;

/*!\class TolmanArrow gcp/tolman-arrow.h
\brief the representation of a curved arrow inside a Tolman loop.

The TolmanArrow class represents arrows inside catalytic loops using Tolman's
conventions.
*/
class TolmanArrow: public gcu::Object
{
public:
/*!
The default constructor.
*/
	TolmanArrow ();
/*!
The destructor.
*/
	virtual ~TolmanArrow ();
};

}   //  namespace gcc

#endif  //  GCP_TOLMAN_ARROW_H
