// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * vector.cc 
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

#include "config.h"
#include "vector.h"
#include <cmath>

namespace gcu
{

// Vector class

Vector::Vector ()
{
	m_X = m_Y = m_Z = 0.;
}

Vector::Vector (double x, double y, double z)
{
	m_X = x;
	m_Y = y;
	m_Z = z;
}

Vector::Vector (double d[3])
{
	m_X = d[0];
	m_Y = d[1];
	m_Z = d[2];
}

Vector::Vector (Vector const &v)
{
	m_X = v.m_X;
	m_Y = v.m_Y;
	m_Z = v.m_Z;
}

Vector Vector::operator+ (Vector const &v) const
{
	Vector res;
	res.m_X = m_X + v.m_X;
	res.m_Y = m_Y + v.m_Y;
	res.m_Z = m_Z + v.m_Z;
	return res;
}

double Vector::operator[] (unsigned i) const
{
	switch (i) {
	case 0:
		return m_X;
	case 1:
		return m_Y;
	case 2:
		return m_Z;
	default:
		return 0.;
	}
}

Vector3f::Vector3f ():
	m_x (0.),
	m_y (0.),
	m_z (0.)
{
}

Vector3f::Vector3f (float x, float y, float z):
	m_x (x),
	m_y (y),
	m_z (z)
{
}

Vector3f::~Vector3f ()
{
}

void Vector3f::normalize ()
{
	double norm = sqrtf (m_x * m_x + m_y * m_y + m_z * m_z);
	m_x /= norm;
	m_y /= norm;
	m_z /= norm;
}

Vector3f &Vector3f::operator= (Vector3f const& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_z = other.m_z;
	return *this;
}

Vector3f Vector3f::operator- (Vector3f const& other) const
{
	Vector3f v (m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
	return v;
}

Vector3f Vector3f::operator+ (Vector3f const& other) const
{
	Vector3f v (m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
	return v;
}

Vector3f Vector3f::operator* (float f) const
{
	Vector3f v (m_x * f, m_y * f, m_z * f);
	return v;
}

Vector3f operator* (float f, Vector3f const& v)
{
	return v * f;
}

};
