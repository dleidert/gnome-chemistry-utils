// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * matrix.h 
 *
 * Copyright (C) 2000-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
 
#ifndef GCU_MATRIX_H
#define GCU_MATRIX_H

namespace gcu
{

/*!\enum MatrixType
This enumeration is used to determine the type a Matrix.
Possible values are:
	- euler: matrix used to find absolute positions from the position inside the solid using Euler's angles.
	- antieuler: the inverse of euler.
	- rotation: matrix representing the rotation to apply to the solid.
	.
This enumeration is used in one of the constructors (Matrix(Psi, Theta, Phi, Type)) of class Matrix.
*/	

enum MatrixType {euler, antieuler, rotation};

/*!\class Matrix gcu/matrix.h
This class provides some operations related to rotation of 	a solid in space. The matrices are 3x3.
*/
class Matrix  
{
public:

/*!
Default constructor. Members are not initialized. This constructor is rarely used.
*/
	Matrix ();
/*!
The destructor of Matrix.
*/
	virtual ~Matrix ();
/*!
@param Psi: precession angle.
@param Theta: nutaton angle.
@param Phi: rotation angle.
@param Type: the type of the Matrix.

Constructs a Matrix instance starting from three angles and the type. if Type is euler or antieuler, the angles are the
Euler's angles. If Type is rotation, the values have a somewhat different meaning: they are the angles rotation describing
the rotation applid to a solid; the result ing matrix is then multiplied by the current "euler" matrix to give th new euler
matrix.
The code used in CrystalView::Rotate and in GtkChem3DViewer code is (when mouse has moved by x and y on the screen):
\code
	gdouble z = sqrt(x*x + y*y);
	Matrix Mat(0, (y > 0)? - acos(x/z) :acos(x/z), z * M_PI / 90., rotation);
	m_Euler = Mat * m_Euler;
\endcode
The (M_PI / 90.) factor is arbitrary here.
*/
	Matrix (double Psi, double Theta, double Phi, MatrixType Type);
/*!
@param x11: value to use at first line and first column of the matrix.
@param x12: value to use at first line and second column of the matrix.
@param x13: value to use at first line and third column of the matrix.
@param x21: value to use at second line and first column of the matrix.
@param x22: value to use at second line and second column of the matrix.
@param x23: value to use at second line and third column of the matrix.
@param x31: value to use at third line and first column of the matrix.
@param x32: value to use at third line and second column of the matrix.
@param x33: value to use at third line and third column of the matrix.

Constructs a matrix from its components.
*/
	Matrix (double x11, double x12, double x13, double x21, double x22, double x23, double x31, double x32, double x33);
/*!
@param cMat: a Matrix instance to use in the multiplication.

The matricial multiplication operator.
*/
	Matrix& operator* (Matrix& cMat);
/*!
@param cMat: the Matrix instance to copy.

Copies a Matrix instance into another one.
*/
	Matrix& operator= (Matrix& cMat);
/*!
@param Psi: precession angle.
@param Theta: nutaton angle.
@param Phi: rotation angle.

Get the Euler's angles associated to a "euler" Matrix as defined in MatrixType.
*/
	void Euler (double& Psi, double& Theta, double& Phi);
/*!
@param dx: the x coordinate.
@param dy: the y coordinate.
@param dz: the z coordinate.

Initially, dx, dy and dz are the components of the vector to transform (multiply) by the matrix and
after execution of this method, dx, dy and dz are the components of the transformed vector. So initial values are lost.
*/
	void Transform (double &dx, double &dy , double &dz);
	
private:
	double x[3][3];
};

}	//namespace gcu

#endif //GCRYSTAL_MATRIX_H
