// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/matrix.h 
 *
 * Copyright (C) 2000-2003
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
 
#ifndef GCU_MATRIX_H
#define GCU_MATRIX_H

#include <glib.h>

namespace gcu
{

enum MatrixType {euler, antieuler, rotation};

class Matrix  
{
public:

	Matrix();
	virtual ~Matrix();
	Matrix(gdouble Psi, gdouble Theta, gdouble Phi, MatrixType Type);
	Matrix(gdouble x11, gdouble x12, gdouble x13, gdouble x21, gdouble x22, gdouble x23, gdouble x31, gdouble x32, gdouble x33);
	Matrix& operator*(Matrix& cMat);
	Matrix& operator=(Matrix& cMat);
	void Euler(gdouble& Psi, gdouble& Theta, gdouble& Phi);
	void Transform(gdouble &dx, gdouble &dy , gdouble &dz);
	
private :
	gdouble x[3][3];
};

}	//namespace gcu

#endif //GCRYSTAL_MATRIX_H
