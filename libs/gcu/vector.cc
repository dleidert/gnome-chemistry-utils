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

};
