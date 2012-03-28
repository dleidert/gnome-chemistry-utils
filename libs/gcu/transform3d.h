// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * transform3d.h - Handle 3D transformations in space groups.
 *
 * Copyright (C) 2007-2010 by Jean Bréfort
 *
 * This file was originally part of the Open Babel project.
 * For more information, see <http://openbabel.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef GCU_TRANSFORM_3D_H
#define GCU_TRANSFORM_3D_H

#include "matrix.h"
#include "vector.h"
#include <list>
#include <string>

namespace gcu
{

/*!\class Transform3d gcu/transform3d.h
@brief Handle 3D transformations, such as space group definitions
@since version 0.12
@sa SpaceGroup

Describes a spatial trnasformation obtained by the combination of a symmetry
operation and a translation.
*/
class Transform3d: private Matrix, private Vector
{
public:
/*!
The default constructor. Initializes all coordinates to 0.
*/
	Transform3d ();
/*!
@param m a symmetry operation matrix.
@param v a translation vector.

Constructs a new Transform3d from its symmetry operation matrix and its
translation vector.
*/
	Transform3d (Matrix const &m, Vector const &v);
/*!
@param s a scalar.

Constructs a new Transform3d from a scalar matrix and a null translation vector.
*/
	Transform3d (double s);
/*!
@param row1 first matrix row.
@param row2 second matrix row.
@param row3 third matrix row.
@param translation the translation vector.

Constructs a new Transform3d from the three rows of its symmetry operation
matrix and its translation vector.
*/
	Transform3d (Vector row1, Vector row2, Vector row3, Vector translation);
/*!
@param d the coefficients of a symmetry operation matrix.
@param t the coefficients of a translation vector.

Constructs a new Transform3d from the coefficients of its symmetry operation
matrix and its translation vector.
*/
	Transform3d (double d[3][3], double t[3]);

/*!
The destructor.
*/
	virtual ~Transform3d ();

/*!
@param v the vector to transform.

Transforms a vector (multiply by the symmetry matrix and add the
translation vector).
@return the transformed vector.
*/
	Vector operator* (Vector const &v) const;

/*!
@param t a Transform3d.

Combines two Transform3d instances a vector.
@return the combination of the two transforms.
*/
	Transform3d operator* (Transform3d const &t) const;

/*!
@return the transform dexcription in CIF convention, such as "x,y,z".
*/
	std::string DescribeAsString() const;
/*!
@return the transform description in CML convention, limiting to the twelve
first values, such as "1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0".
*/
	std::string DescribeAsValues() const;

/*!
Ensure that the translation vector coordinates are all in the interval [0,1[.
*/
	void Normalize();
};

}

#endif // GCU_TRANSFORM_3D_H

//! \file transform3d.h
//! \brief Handle 3D transformations in space groups.
