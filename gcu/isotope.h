// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * isotope.h 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

	IsotopicPattern *Simplify (void);
	IsotopicPattern *Multiply (IsotopicPattern& pattern);
	IsotopicPattern *Square (void);
	void Copy (IsotopicPattern& pattern);

	void SetValue (int A, double percent);
	void Normalize ();
	void Clear ();
	void Ref () {ref_count++;}
	void Unref ();
	int GetMinMass () {return m_min;}
	int GetMonoNuclNb () {return m_mono;}
	double GetMonoMass () {return m_mono_mass;}
	void SetMonoMass (double mass);
	int GetValues (double **values);

private:
	int m_min, m_max, m_mono;
	int ref_count;
	vector<double> m_values;
	double m_mono_mass;
	static double epsilon;
};

}
#endif	// GCU_ISOTOPE_H
