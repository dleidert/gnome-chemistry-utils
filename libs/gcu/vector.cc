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

#include "vector.h"
#include <cmath>

namespace gcu
{

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

Vector3d::Vector3d ():
	m_x (0.),
	m_y (0.),
	m_z (0.)
{
}

Vector3d::Vector3d (double x, double y, double z):
	m_x (x),
	m_y (y),
	m_z (z)
{
}

Vector3d::~Vector3d ()
{
}

void Vector3d::normalize ()
{
	double norm = sqrt (m_x * m_x + m_y * m_y + m_z * m_z);
	m_x /= norm;
	m_y /= norm;
	m_z /= norm;
}

double Vector3d::norm ()
{
	return sqrt (m_x * m_x + m_y * m_y + m_z * m_z);
}

Vector3d &Vector3d::operator= (Vector3d const& other)
{
	m_x = other.m_x;
	m_y = other.m_y;
	m_z = other.m_z;
	return *this;
}

Vector3d Vector3d::operator- (Vector3d const& other) const
{
	Vector3d v (m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
	return v;
}

Vector3d Vector3d::operator+ (Vector3d const& other) const
{
	Vector3d v (m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
	return v;
}

Vector3d Vector3d::operator* (double d) const
{
	Vector3d v (m_x * d, m_y * d, m_z * d);
	return v;
}

Vector3d &Vector3d::operator*= (double d)
{
	m_x *= d;
	m_y *= d;
	m_z *= d;
	return *this;
}

Vector3d Vector3d::operator/ (double d) const
{
	Vector3d v (m_x / d, m_y / d, m_z / d);
	return v;
}

Vector3d &Vector3d::operator/= (double d)
{
	m_x /= d;
	m_y /= d;
	m_z /= d;
	return *this;
}

Vector3d &Vector3d::loadOrtho (Vector3d const &other)
{
	if (!(other.m_x < other.m_z * 1.e-11)
	|| !(other.m_y < other.m_z * 1.e-11))
	{
		double nm = sqrt (other.m_x * other.m_x + other.m_y * other.m_y);
		m_x = - other.m_y / nm;
		m_y = other.m_x / nm;
		m_z = 0.;
	} else {
		/* if both x and y are close to zero, then the vector is close
		* to the z-axis, so it's far from colinear to the x-axis for instance.
		* So we take the crossed product with (1,0,0) and normalize it.
		*/
		double nm = sqrt (other.m_y * other.m_y + other.m_z * other.m_z);
		m_x = 0.;
		m_y = - other.m_z / nm;
		m_z = other.m_y / nm;
	}
	return *this;
}

Vector3d Vector3d::ortho () const
{
	Vector3d v;
	v.loadOrtho (*this);
	return v;
}

Vector3d Vector3d::cross (Vector3d const &other) const
{
	Vector3d v;
	v.m_x = m_y * other.m_z - m_z * other.m_y;
	v.m_y = m_z * other.m_x - m_x * other.m_z;
	v.m_z = m_x * other.m_y - m_y * other.m_x;
	return v;
}

void Vector3d::cross (Vector3d const &other, Vector3d *res) const
{
	res->m_x = m_y * other.m_z - m_z * other.m_y;
	res->m_y = m_z * other.m_x - m_x * other.m_z;
	res->m_z = m_x * other.m_y - m_y * other.m_x;
}

};
