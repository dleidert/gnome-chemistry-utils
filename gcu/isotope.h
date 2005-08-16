// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * isotope.h 
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

#ifndef GCU_ISOTOPE_H
#define GCU_ISOTOPE_H

#include <gcu/chemistry.h>
#include <vector>

using namespace std;

namespace gcu
{

class Isotope: public GcuIsotope
{
public:
	Isotope ();
	~Isotope ();
};

class IsotopicPattern
{
public:
	IsotopicPattern ();
	IsotopicPattern (int min, int max);
	~IsotopicPattern ();

	IsotopicPattern *Simplify ();
	IsotopicPattern *multiply (IsotopicPattern& pattern);
	IsotopicPattern *square (void);

	void SetValue (int A, double percent);
	void Normalize ();
	void Ref () {ref_count++;}
	void Unref ();

private:
	int m_min, m_max, m_mono;
	int ref_count;
	vector<double> m_values;
	static double epsilon;
};

}
#endif	// GCU_ISOTOPE_H
