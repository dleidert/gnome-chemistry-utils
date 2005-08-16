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
	m_min = m_max = m_mono = 0;
	ref_count = 1;
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
	m_mono = 0;
	m_values.resize (max - min + 1);
	ref_count = 1;
}

IsotopicPattern::~IsotopicPattern ()
{
}

double IsotopicPattern::epsilon = 1e-6;

IsotopicPattern *IsotopicPattern::Simplify ()
{
puts("1.0");
	int min = 0, max = m_max - m_min;
	int i, j, imax = max + 1;
	double vmax = m_values[0], minval;
	for (i = 1; i < imax; i++)
		if (m_values[i] > vmax) {
			vmax = m_values[i];
		}
	minval = epsilon * vmax;
	while (m_values[min] < minval)
		min++;
	while (m_values[max] < minval)
		max--;
printf("min=%d max=%d\n",min,max);
	IsotopicPattern *pat = new IsotopicPattern (min, max);
puts("1.2");
	pat->m_mono = m_mono;
	for (i = min, j = 0; i <= max; i++, j++)
		pat->m_values[j] = m_values[i];
	return pat;
}

IsotopicPattern *IsotopicPattern::multiply (IsotopicPattern &pattern)
{
	IsotopicPattern *pat = new IsotopicPattern (m_min + pattern.m_min, m_max + pattern.m_max);
	return pat;
}

IsotopicPattern *IsotopicPattern::square ()
{
	IsotopicPattern *pat = new IsotopicPattern (2 * m_min, 2 * m_max);
	pat->m_mono = 2 * m_mono;
	int i, j, k, imax = pat->m_max - pat->m_min + 1;
	for (i = 0; i < imax; i++) {
		pat->m_values[i] = 0.;
		for (j = max (0, m_max - i), k = min (j, m_max - j); k > j; k--, j++) {
			pat->m_values[i] += 2. * m_values[k] * m_values[j];
		}
		if (j == k)
			pat->m_values[i] += m_values[j] * m_values[j];
	}
	return pat;
}

void IsotopicPattern::SetValue (int A, double percent)
{
	if (A >= m_min && A <= m_max) {
		A -= m_min;
#if HAS_VECTOR_AT
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
	double max = m_values[0];
	int i, maxi = m_max - m_min + 1;
	m_mono = 0;
	for (i = 1; i < maxi; i++)
		if (m_values[i] > max) {
			m_mono = i;
			max = m_values[i];
		}
	m_mono += m_min;
	for (i = 1; i < maxi; i++)
		m_values[i] /= max;
}

void IsotopicPattern::Unref ()
{
	ref_count--;
	if (!ref_count)
		delete this;
}
