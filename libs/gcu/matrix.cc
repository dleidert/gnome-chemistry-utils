// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/matrix.cc
 *
 * Copyright (C) 2000-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "matrix.h"
#include "vector.h"
#include <cmath>

namespace gcu
{

Matrix::Matrix ()
{
	x[0][0] = x[1][1] = x[2][2] = 1.;
	x[0][1] = x[0][2] = x[1][0] = x[1][2] = x[2][0] = x[2][1] = 0.;
}

Matrix::Matrix (double d)
{
	x[0][0] = x[1][1] = x[2][2] = d;
	x[0][1] = x[0][2] = x[1][0] = x[1][2] = x[2][0] = x[2][1] = 0.;
}

Matrix::Matrix (double d[3][3])
{
	x[0][0] = d[0][0];
	x[0][1] = d[0][1];
	x[0][2] = d[0][2];
	x[1][0] = d[1][0];
	x[1][1] = d[1][1];
	x[1][2] = d[1][2];
	x[2][0] = d[2][0];
	x[2][1] = d[2][1];
	x[2][2] = d[2][2];
}

Matrix::Matrix (Matrix const &m)
{
	x[0][0] = m.x[0][0];
	x[0][1] = m.x[0][1];
	x[0][2] = m.x[0][2];
	x[1][0] = m.x[1][0];
	x[1][1] = m.x[1][1];
	x[1][2] = m.x[1][2];
	x[2][0] = m.x[2][0];
	x[2][1] = m.x[2][1];
	x[2][2] = m.x[2][2];
}

Matrix::Matrix (Vector const &v1, Vector const &v2, Vector const &v3, bool as_rows)
{
	if (as_rows) {
		x[0][0] = v1.GetX ();
		x[0][1] = v1.GetY ();
		x[0][2] = v1.GetZ ();
		x[1][0] = v2.GetX ();
		x[1][1] = v2.GetY ();
		x[1][2] = v2.GetZ ();
		x[2][0] = v3.GetX ();
		x[2][1] = v3.GetY ();
		x[2][2] = v3.GetZ ();
	} else {
		x[0][0] = v1.GetX ();
		x[0][1] = v2.GetX ();
		x[0][2] = v3.GetX ();
		x[1][0] = v1.GetY ();
		x[1][1] = v2.GetY ();
		x[1][2] = v3.GetY ();
		x[2][0] = v1.GetZ ();
		x[2][1] = v2.GetZ ();
		x[2][2] = v3.GetZ ();
	}
}

Matrix::~Matrix ()
{
}

Matrix::Matrix (double Psi, double Theta, double Phi, MatrixType Type)
{
	double sp = sin (Psi);
	double cp = cos (Psi);
	double st = sin (Theta);
	double ct = cos (Theta);
	double sf = sin (Phi);
	double cf = cos (Phi);
	switch(Type) {
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

Matrix::Matrix (double x11, double x12, double x13, double x21, double x22, double x23, double x31, double x32, double x33)
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

Matrix& Matrix::operator* (Matrix const &cMat) const
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

Matrix& Matrix::operator= (Matrix const &cMat)
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

void Matrix::Euler (double& Psi, double& Theta, double& Phi)
{
	if (fabs(x[2][2]) > .999999999) {
		Theta = (x[2][2] > 0) ? 0 : 3.1415926535897931;
		Psi = 0;
		if (fabs(x[0][0]) > .999999999)
		Phi = (x[0][0] > 0) ? 0 : 3.1415926535897931;
		else Phi = (x[1][0] > 0) ? acos(x[0][0]) : - acos(x[0][0]);
	} else {
		Theta = acos(x[2][2]);
		double st = sin(Theta);
		double si = x[0][2] / st;
		double co = - x[1][2] / st;
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

void Matrix::Transform (double &dx, double &dy , double &dz) const
{
	double x1 = dx * x[0][0] + dy * x[0][1] + dz * x[0][2];
	double y1 = dx * x[1][0] + dy * x[1][1] + dz * x[1][2];
	dz = dx * x[2][0] + dy * x[2][1] + dz * x[2][2];
	dx = x1;
	dy = y1;
}

Vector Matrix::operator* (Vector const &v) const
{
	Vector r;
	r.SetX (v.GetX () * x[0][0] + v.GetY () * x[0][1] + v.GetZ () * x[0][2]);
	r.SetY (v.GetX () * x[1][0] + v.GetY () * x[1][1] + v.GetZ () * x[1][2]);
	r.SetZ (v.GetX () * x[2][0] + v.GetY () * x[2][1] + v.GetZ () * x[2][2]);
	return r;
}

Vector Matrix::glmult (Vector const &v) const
{
	Vector r;
	r.SetZ (v.GetZ () * x[0][0] + v.GetX () * x[0][1] + v.GetY () * x[0][2]);
	r.SetX (v.GetZ () * x[1][0] + v.GetX () * x[1][1] + v.GetY () * x[1][2]);
	r.SetY (v.GetZ () * x[2][0] + v.GetX () * x[2][1] + v.GetY () * x[2][2]);
	return r;
}

// Implementation of GLMatrix
GLMatrix::GLMatrix ()
{
}

GLMatrix::~GLMatrix ()
{
}

}	//	namespace gcu
