// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * matrix2d.h 
 *
 * Copyright (C) 2004
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
 
#ifndef GCU_MATRIX2D_H
#define GCU_MATRIX2D_H

namespace gcu
{

/*!\class Matrix2D gcu/matrix2d.h
This class provides some operations related to transformations in a plane. The matrices are 2x2.
*/
class Matrix2D  
{
public:

/*!
Default constructor. Members are not initialized. This constructor is rarely used.
*/
	Matrix2D();
/*!
The destructor of Matrix.
*/
	virtual ~Matrix2D();
/*!
@param Angle: rotation angle.
@param Deg: if true, Angle is expressed in degrees, otherwise in radians.
Default is true.

Constructs a Matrix2D representing a rotation.
*/
	Matrix2D(double Angle, bool Deg = true);
/*!
@param x11: value to use at first line and first column of the matrix.
@param x12: value to use at first line and second column of the matrix.
@param x21: value to use at second line and first column of the matrix.
@param x22: value to use at second line and second column of the matrix.

Constructs a matrix from its components.
*/
	Matrix2D(double x11, double x12, double x21, double x22);
/*!
@param cMat: a Matrix2D instance to use in the multiplication.

The matricial multiplication operator.
*/
	Matrix2D& operator*(Matrix2D& cMat);
/*!
@param cMat: the Matrix2D instance to copy.

Copies a Matrix instance into another one.
*/
	Matrix2D& operator=(Matrix2D& cMat);
/*!
@param dx: the x coordinate.
@param dy: the y coordinate.
@param dz: the z coordinate.

Initially, dx and dy are the components of the vector to transform (multiply) by the matrix and
after execution of this method, dx, dy and dz are the components of the transformed vector. So initial values are lost.
*/
	void Transform(double &dx, double &dy);
	
private :
	double x[2][2];
};

}	//namespace gcu

#endif //GCRYSTAL_MATRIX2D_H
