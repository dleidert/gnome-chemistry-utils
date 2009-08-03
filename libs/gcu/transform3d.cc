// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * Transform3d.h - Handle 3D transformations in space groups.
 *  
 * Copyright (C) 2007-2009 by Jean Bréfort
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

#include "config.h"
#include "matrix.h"
#include "transform3d.h"
#include "vector.h"
#include <sstream>

using namespace std;

namespace gcu
{

Transform3d::Transform3d ():
	Matrix (),
	Vector ()
{
}

Transform3d::Transform3d (Matrix const &m, Vector const &v):
	Matrix (m),
	Vector (v)
{
	Normalize ();
}

Transform3d::Transform3d (double s):
	Matrix (s),
	Vector ()
{
}

Transform3d::Transform3d (Vector row1, Vector row2, Vector row3, Vector translation):
	Matrix (row1, row2, row3),
	Vector (translation)
{
	Normalize ();
}

Transform3d::Transform3d (double d[3][3], double t[3]):
	Matrix (d),
	Vector (t)
{
	Normalize ();
}

Transform3d::~Transform3d ()
{
}

Vector Transform3d::operator* (Vector const &v) const
{
	return *static_cast <Matrix const *> (this) * v + *static_cast <Vector const *> (this);
}

Transform3d Transform3d::operator* (Transform3d const &t) const
{
	return Transform3d (*static_cast <Matrix const *> (this) * *static_cast <Matrix const *> (&t), *this * *static_cast <Vector const *> (&t));
}

/*! 
*/
string Transform3d::DescribeAsString() const
{
	ostringstream r;
	int n, i, j;
	const Matrix *m = static_cast <const Matrix *> (this);
	const Vector *v = static_cast <const Vector *> (this);
	bool neg, first;
	for (i = 0; i < 3; i++) {
		if (i)
			r << ",";
		n = floor ((*v)[i] * 12.0 + 0.1);
		j = 0;
		while ((*m)(i, j) == 0.)
			j++;
		neg = (*m)(i, j) < 0.;
		switch (n) {
		case 2:
			r << ((neg)? "1/6": "1/6+");
			break;
		case 3:
			r << ((neg)? "1/4": "1/4+");
			break;
		case 4:
			r << ((neg)? "1/3": "1/3+");
			break;
		case 6:
			r << ((neg)? "1/2": "1/2+");
			break;
		case 8:
			r << ((neg)? "2/3": "2/3+");
			break;
		case 9:
			r << ((neg)? "3/4": "3/4+");
			break;
		case 10:
			r << ((neg)? "5/6": "5/6+");
			break;
		}
		first = true;
		while (j < 3) {
			if ((*m) (i, j) != 0.) {
				neg = (*m) (i, j) < 0.;
				switch (j) {
				case 0:
					r << ((neg)? "-x": (first? "x": "+x"));
					break;
				case 1:
					r << ((neg)? "-y": (first? "y": "+y"));
					break;
				case 2:
					r << ((neg)? "-z": (first? "z": "+z"));
					break;
				}
				first = false;
			}
			j++;
		}
	}
	return r.str ();
}

string Transform3d::DescribeAsValues() const
{
	ostringstream oss;
	const Matrix *m = static_cast <const Matrix *> (this);
	const Vector *v = static_cast <const Vector *> (this);
	oss << (*m) (0,0) << " " << (*m) (0,1) << " " << (*m) (0,2) << " " << v->GetX () << " ";
	oss << (*m) (1,0) << " " << (*m) (1,1) << " " << (*m) (1,2) << " " << v->GetY () << " ";
	oss << (*m) (2,0) << " " << (*m) (2,1) << " " << (*m) (2,2) << " " << v->GetZ ();
	return oss.str ();
}

void Transform3d::Normalize()
{
	Vector *vv = static_cast <Vector *> (this);
	vv->GetRefX () -= floor (vv->GetX () + .01); /* .01 should work in all cases in this context */
	vv->GetRefY () -= floor (vv->GetY () + .01);
	vv->GetRefZ () -= floor (vv->GetZ () + .01);
}

}   //  namespace gcu
