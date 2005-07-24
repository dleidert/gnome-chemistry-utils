// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * isotope.cc 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.rg>
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

#include "config.h"
#include "isotope.h"

using namespace gcu;

Isotope::Isotope ()
{
	A = 0;
	name = NULL;
	abundance.value = 0.;
	abundance.prec = 0;
	abundance.delta = 0;
	mass.value = 0.;
	mass.prec = 0;
	mass.delta = 0;
	spin = 0;
	decay_modes = NULL;
	decay_period.value = 0.;
	decay_period.prec = 0;
	decay_period.unit = NULL;
}

Isotope::~Isotope ()
{
	if (name != NULL)
		g_free (name);
	if (decay_modes != NULL)
		g_free (decay_modes);
	if (decay_period.unit != NULL)
		g_free (decay_period.unit);
}

IsotopicPattern::IsotopicPattern ()
{
	m_min = m_max = m_nIsotopes = m_mono = 0;
}

IsotopicPattern::IsotopicPattern (int min, int max)
{
	if (max >= min) {
		m_min = min;
		m_max = max;
	} else {
		m_max = min;
		m_min = max;
	}
	m_nIsotopes = m_mono = 0;
	m_values.resize (max - min + 1);
}

IsotopicPattern::~IsotopicPattern ()
{
}

IsotopicPattern& IsotopicPattern::operator= (IsotopicPattern& pattern)
{
	return *this;
}

IsotopicPattern IsotopicPattern::operator^ (int n)
{
	IsotopicPattern pat;
	return pat;
}

IsotopicPattern IsotopicPattern::operator* (IsotopicPattern& pattern)
{
	IsotopicPattern pat;
	return pat;
}

void IsotopicPattern::SetValue (int A, double percent)
{
	if (A >= m_min && A <= m_max) {
		A -= m_min;
#if HAS_VECTOR_AT
printf("setting value %g at position %d\n",percent,A);
		m_values.at (A) = percent;
#else
		vector<double>::iterator it;
		it = m_values.begin ();
		it += A;
		m_values.insert (it, percent);
#endif
	}
}

void IsotopicPattern::Normalize ()
{
}
