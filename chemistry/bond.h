// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * bond.h 
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

#ifndef GCU_BOND_H
#define GCU_BOND_H

#include <list>
#include "object.h"

using namespace std;

namespace gcu
{

class Atom;


class Bond: public Object
{
public:
	Bond();
	Bond(Atom* first, Atom* last, unsigned char order);
	virtual ~Bond();
	
	virtual Atom* GetAtom(int which); //0 = first, 1 = last, others reserved (for multicentered bonds?)
	virtual Atom* GetAtom(Atom* pAtom, int which = 0);	//which is just a place holder for multicenter bonds; returns an atom different from pAtom
															//i.e. the other end of the bond
	unsigned char GetOrder();
	void SetOrder(unsigned char Order);
	virtual xmlNodePtr Save(xmlDocPtr xml);
	virtual bool Load(xmlNodePtr);
	virtual void IncOrder(int n = 1);
	virtual bool LoadNode(xmlNodePtr);
	virtual bool SaveNode(xmlDocPtr, xmlNodePtr);

protected:
	unsigned char m_order;
	Atom *m_Begin, *m_End;// only 2 centers bonds, other bonds should be covered by derived classes
};

} // namespace gcu

#endif // GCU_BOND_H
