// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * element.h 
 *
 * Copyright (C) 2002-2003
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

#ifndef GCU_ELEMENT_H
#define GCU_ELEMENT_H

#include <glib.h>
#include <string>
#include <vector>
#include "chemistry.h"

using namespace std;

class EltTable;

namespace gcu
{

class Element
{
friend class EltTable;
public:
	Element(int Z, const char* Symbol);
	virtual ~Element();
	
	static const gchar* Symbol(gint Z);
	static bool BestSide(gint Z);
	static gint Z(const gchar* name);
	static Element* GetElement(gint Z);
	static Element* GetElement(const gchar* name);
	static bool GetRadius(GcuAtomicRadius* radius);
	static bool GetElectronegativity(GcuElectronegativity* en);

	int GetZ() {return m_Z;}
	const char* GetSymbol() {return m_Symbol;}
	char GetDefaultValence() {return m_DefaultValence;}
	bool GetBestSide() {return m_BestSide;}
	double* GetDefaultColor() {return m_DefaultColor;}
	const char* GetName() {return name.c_str();}
	const GcuAtomicRadius** GetRadii();
	const GcuElectronegativity** GetElectronegativities();
	
private:
	unsigned char m_Z;
	char m_Symbol[4];
	char m_DefaultValence;
	bool m_BestSide;
	double m_DefaultColor[3];
	string name;
	vector<GcuAtomicRadius*> m_radii;
	vector<GcuElectronegativity*> m_en;
};

} // namespace gcu

#endif // GCU_ELEMENT_H
