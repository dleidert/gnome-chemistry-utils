// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/matrix.cc 
 *
 * Copyright (C) 2000-2002
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

#include <glib.h>
#include <math.h>
#include "matrix.h"

using namespace gcu;

Matrix::Matrix()
{
}

Matrix::~Matrix()
{
}

Matrix::Matrix(gdouble Psi, gdouble Theta, gdouble Phi, MatrixType Type)
{
	gdouble sp = sin(Psi);
	gdouble cp = cos(Psi);
	gdouble st = sin(Theta);
	gdouble ct = cos(Theta);
	gdouble sf = sin(Phi);
	gdouble cf = cos(Phi);
	switch(Type)
	{
	case euler :
		x[0][0] = cf * cp - sf * sp * ct;
		x[0][1] = - cp * sf - sp * cf * ct;
		x[0][2] = st * sp;
		x[1][0] =  sp * cf + cp * sf * ct;
		x[1][1] = cf * cp * ct - sf * sp;
		x[1][2] = - st * cp;
		x[2][0] = st * sf;
		x[2][1] = st * cf;
		x[2][2] = ct;
		break;
	case antieuler :
		x[0][0] = cf * cp - sf * sp * ct;
		x[0][1] = cf * sp + cp * sf * ct;
		x[0][2] = st * sf;
		x[1][0] = - sf * cp - cf * sp * ct;
		x[1][1] = cf * cp * ct - sf * sp;
		x[1][2] = st * cf;
		x[2][0] = st * sp;
		x[2][1] = - st * cp;
		x[2][2] = ct;
		break;
	case rotation :
		Matrix m1(Psi,Theta,Phi,euler);
		Matrix m2(Psi,Theta,0,antieuler);
		*this = m1 * m2; 
		break;
	}
}

Matrix::Matrix(gdouble x11, gdouble x12, gdouble x13, gdouble x21, gdouble x22, gdouble x23, gdouble x31, gdouble x32, gdouble x33)
{
	x[0][0] = x11;
	x[0][1] = x12;
	x[0][2] = x13;
	x[1][0] = x21;
	x[1][1] = x22;
	x[1][2] = x23;
	x[2][0] = x31;
	x[2][1] = x32;
	x[2][2] = x33;
}

Matrix& Matrix::operator*(Matrix& cMat)
{
	static Matrix cMat0;
	Matrix m(
			x[0][0] * cMat.x[0][0] + x[0][1] * cMat.x[1][0] + x[0][2] * cMat.x[2][0],
			x[0][0] * cMat.x[0][1] + x[0][1] * cMat.x[1][1] + x[0][2] * cMat.x[2][1],
			x[0][0] * cMat.x[0][2] + x[0][1] * cMat.x[1][2] + x[0][2] * cMat.x[2][2],
			x[1][0] * cMat.x[0][0] + x[1][1] * cMat.x[1][0] + x[1][2] * cMat.x[2][0],
			x[1][0] * cMat.x[0][1] + x[1][1] * cMat.x[1][1] + x[1][2] * cMat.x[2][1],
			x[1][0] * cMat.x[0][2] + x[1][1] * cMat.x[1][2] + x[1][2] * cMat.x[2][2],
			x[2][0] * cMat.x[0][0] + x[2][1] * cMat.x[1][0] + x[2][2] * cMat.x[2][0],
			x[2][0] * cMat.x[0][1] + x[2][1] * cMat.x[1][1] + x[2][2] * cMat.x[2][1],
			x[2][0] * cMat.x[0][2] + x[2][1] * cMat.x[1][2] + x[2][2] * cMat.x[2][2]
		);
	return cMat0 = m;
}

Matrix& Matrix::operator=(Matrix& cMat)
{
	x[0][0] = cMat.x[0][0];
	x[0][1] = cMat.x[0][1];
	x[0][2] = cMat.x[0][2];
	x[1][0] = cMat.x[1][0];
	x[1][1] = cMat.x[1][1];
	x[1][2] = cMat.x[1][2];
	x[2][0] = cMat.x[2][0];
	x[2][1] = cMat.x[2][1];
	x[2][2] = cMat.x[2][2];
	return *this;
}

void Matrix::Euler(gdouble& Psi, gdouble& Theta, gdouble& Phi)
{
	if (fabs(x[2][2]) > .999999999)
	{
		Theta = (x[2][2] > 0) ? 0 : 3.1415926535897931;
		Psi = 0;
		if (fabs(x[0][0]) > .999999999)
		Phi = (x[0][0] > 0) ? 0 : 3.1415926535897931;
		else Phi = (x[1][0] > 0) ? acos(x[0][0]) : - acos(x[0][0]);
	}
	else
	{
		Theta = acos(x[2][2]);
		gdouble st = sin(Theta);
		gdouble si = x[0][2] / st;
		gdouble co = - x[1][2] / st;
		if (fabs(co) > .999999999)
			Psi = (co > 0) ? 0 : 3.1415926535897931;
		else
			Psi = (si > 0) ? acos(co) : - acos(co);
		si = x[2][0] / st;
		co = x[2][1] / st;
		if (fabs(co) > .999999999)
			Phi = (co > 0) ? 0 : 3.1415926535897931;
		else
			Phi = (si > 0) ? acos(co) : - acos(co);
	}
}

void Matrix::Transform(gdouble &dx, gdouble &dy , gdouble &dz)
{
	gdouble x1 = dx * x[0][0] + dy * x[0][1] + dz * x[0][2];
	gdouble y1 = dx * x[1][0] + dy * x[1][1] + dz * x[1][2];
	dz = dx * x[2][0] + dy * x[2][1] + dz * x[2][2];
	dx = x1;
	dy = y1;
}
 
