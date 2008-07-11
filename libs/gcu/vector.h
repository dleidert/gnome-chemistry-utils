// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * vector.h 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 
#ifndef GCU_VECTOR_H
#define GCU_VECTOR_H

#include "macros.h"

namespace gcu
{

/*! class Vector3f gcu/vector.h
*/

class Vector3f
{
public:
	Vector3f ();
	Vector3f (float x, float y, float z);
	~Vector3f ();

	void normalize ();
	Vector3f &operator= (Vector3f const& other);
	Vector3f operator- (Vector3f const& other) const;
	Vector3f operator+ (Vector3f const& other) const;
	Vector3f operator* (float f) const;

private:
	GCU_PROP (float, x);
	GCU_PROP (float, y);
	GCU_PROP (float, z);
};

Vector3f operator* (float f, Vector3f const& v);

}	//namespace gcu

#endif //GCU_VECTOR_H
