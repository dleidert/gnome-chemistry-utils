// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystalcleavage.h 
 *
 * Copyright (C) 2002-2004
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

#ifndef CRYSTAL_CLEAVAGE_H
#define CRYSTAL_CLEAVAGE_H

#include <libxml/parser.h>
#include <list>

namespace gcu
{

/*! \class CrystalCleavage gcu/crystalcleavage.h
Represents cleavages of a cystal along a plane defined by its Miller indices. A defined number
of planes are removed by each cleavage. These are those for which the value hx + ky + lz are the greatest. To
remove planes frome the other side of the crystal, change the sign of the three Miller indices.
*/
class CrystalCleavage
{
public:
/*
The default constructor.
*/
	CrystalCleavage();
/*!
@param ccCleavage: the cleavage instance to copy.

The copy constructor.
*/
	CrystalCleavage(CrystalCleavage& ccCleavage) ;
/*
The destructor of CrystalAtom.
*/
	virtual ~CrystalCleavage();

/*!
@return the number of planes cleaved.
*/
	int& Planes() {return m_nPlanes ;}
/*!
@return the Miller's h index of the cleavage plane.
*/
	int &h() {return m_nh ;}
/*!
@return the Miller's k index of the cleavage plane.
*/
	int &k() {return m_nk ;}
/*!
@return the Miller's l index of the cleavage plane.
*/
	int &l() {return m_nl ;}
/*!
@param ccCleavage: the cleavage to copy.

@return a CrystalCleavage identical to ccCleavage.
*/
	CrystalCleavage& operator=(CrystalCleavage& ccCleavage) ;
/*!
@param ccCleavage: a CrystalCleavage instance.
@return true if the two cleavages have the same Miller indices and false otherwise.
*/
	bool operator==(CrystalCleavage& ccCleavage) ;
/*!
	@param xml: the xmlDoc used to save the document.
	
	Used to save the cleavage to the xmlDoc. Each serializable Object should implement this virtual method.
	@return the xmlNode containing the serialized cleavage.
*/
	xmlNodePtr Save(xmlDocPtr xml);
/*!
@param node: a pointer to the xmlNode containing the serialized cleavage.

Used to load an CrystalCleavage instance in memory. The CrystalCleavage must already exist.
@return true on succes, false otherwise.
*/
	bool Load(xmlNodePtr node);
	
protected:
/*!
Miller's h index.
*/
	int m_nh;
/*!
Miller's k index.
*/
	int m_nk;
/*!
Miller's l index.
*/
	int m_nl;
/*!
Number of planes cleaved.
*/
	int m_nPlanes ;
};

/*!\class CrystalClevageList
a list of pointers to CrystalCleavage instances derived from std::list.
*/
typedef std::list<CrystalCleavage*> CrystalCleavageList;
	
} //namespace gcu

#endif //CRYSTAL_CLEAVAGE_H
