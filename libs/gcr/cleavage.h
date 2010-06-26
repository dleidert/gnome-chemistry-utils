// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * gcr/cleavage.h 
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_CLEAVAGE_H
#define GCR_CLEAVAGE_H

#include <libxml/parser.h>
#include <list>

/*!\file*/
namespace gcr
{

/*! \class Cleavage gcr/cleavage.h
Represents cleavages of a cystal along a plane defined by its Miller indices. A defined number
of planes are removed by each cleavage. These are those for which the value hx + ky + lz are the greatest. To
remove planes frome the other side of the crystal, change the sign of the three Miller indices.
*/
class Cleavage
{
public:
/*
The default constructor.
*/
	Cleavage ();
/*!
@param ccCleavage: the cleavage instance to copy.

The copy constructor.
*/
	Cleavage (Cleavage& ccCleavage) ;
/*
The destructor of Cleavage.
*/
	virtual ~Cleavage ();

/*!
@return the number of planes cleaved.
*/
	int& Planes () {return m_nPlanes ;}
/*!
@return the Miller's h index of the cleavage plane.
*/
	int& h () {return m_nh ;}
/*!
@return the Miller's k index of the cleavage plane.
*/
	int& k () {return m_nk ;}
/*!
@return the Miller's l index of the cleavage plane.
*/
	int & l () {return m_nl ;}
/*!
@param ccCleavage: the cleavage to copy.

@return a Cleavage identical to ccCleavage.
*/
	Cleavage& operator= (Cleavage& ccCleavage) ;
/*!
@param ccCleavage: a Cleavage instance.
@return true if the two cleavages have the same Miller indices and false otherwise.
*/
	bool operator== (Cleavage& ccCleavage) ;
/*!
	@param xml: the xmlDoc used to save the document.
	
	Used to save the cleavage to the xmlDoc. Each serializable Object should implement this virtual method.
	@return the xmlNode containing the serialized cleavage.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node: a pointer to the xmlNode containing the serialized cleavage.

Used to load an Cleavage instance in memory. The Cleavage must already exist.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
	
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

/*!
a list of pointers to Cleavage instances derived from std::list.
*/
typedef std::list<Cleavage*> CleavageList;
	
} //namespace gcr

#endif	//	GCR_CLEAVAGE_H
