// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/atom.h 
 *
 * Copyright (C) 2002
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

#ifndef GCU_ATOM_H
#define GCU_ATOM_H

#include <map>
#include <glib.h>
#include "object.h"

using namespace std;

namespace gcu
{
	
class Bond;

class Atom: public Object
{
public:
	Atom();
	Atom(int Z, double x, double y, double z);
	Atom(Atom& a);
	Atom& operator=(Atom&);
	virtual ~Atom();

public :
	double Distance(Atom* pAtom);
	void zoom(double ZoomFactor);
	bool GetCoords(double *x, double *y, double *z = NULL);
	void SetCoords(double x, double y, double z = 0) {m_x = x; m_y = y; m_z = z;}
	int GetZ() {return m_Z;}
	virtual void SetZ(int Z);
	const gchar* GetSymbol();
	void AddBond(Bond* pBond);
	void RemoveBond(Bond* pBond);
	double x() {return m_x;}
	double y() {return m_y;}
	double z() {return m_z;}
	Bond* GetFirstBond(map<Atom*, Bond*>::iterator& i);
	Bond* GetNextBond(map<Atom*, Bond*>::iterator& i);
	Bond* GetBond(Atom*);
	int GetBondsNumber() {return m_Bonds.size();}
	virtual xmlNodePtr Save(xmlDocPtr xml);
	virtual bool Load(xmlNodePtr);
	virtual bool LoadNode(xmlNodePtr);
	virtual bool SaveNode(xmlDocPtr, xmlNodePtr);
	virtual void Move(double x, double y, double z = 0);

protected:
	int m_Z;
	double m_x, m_y, m_z;
	map<Atom*, Bond*> m_Bonds;
};

} //namespace gcu
#endif // GCU_ATOM_H
