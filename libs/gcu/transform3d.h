// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * transform3d.h - Handle 3D transformations in space groups.
 *  
 * Copyright (C) 2007-2008 by Jean Bréfort
 * 
 * This file was originally part of the Open Babel project.
 * For more information, see <http://openbabel.sourceforge.net/>
 *  
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
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
*/
class Transform3d: private Matrix, private Vector
{
public:
/*! 
*/
	Transform3d ();
/*! 
*/
	Transform3d (Matrix const &m, Vector const &v);
/*! 
*/
	Transform3d (double s);
/*! 
*/
	Transform3d (Vector row1, Vector row2, Vector row3, Vector translation);
/*! 
*/
	Transform3d (double d[3][3], double t[3]);

/*! 
*/
	virtual ~Transform3d ();

/*! 
*/
	Vector operator* (Vector const &) const;

/*! 
*/
	Transform3d operator* (Transform3d const &) const;

/*! 
*/
	std::string DescribeAsString() const;
/*! 
*/
	std::string DescribeAsValues() const;

/*! 
*/
	void Normalize();
};

}

#endif // GCU_TRANSFORM_3D_H

//! \file transform3d.h 
//! \brief Handle 3D transformations in space groups.
