// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery-arrow.h 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_MESOMERY_ARROW_H
#define GCHEMPAINT_MESOMERY_ARROW_H

/*!file*/
#include "arrow.h"

namespace gcp {

class Mesomery;
class Mesomer;

/*!\class MesomeryArrow gcp/mesomery-arrow.h
Arrow class for double headed arrows used in mesomery relationships.*/
class MesomeryArrow: public Arrow
{
public:
/*!
@param mesomery the parent mesomery relationship if any.

Constructs a mesomery arrow. If \a mesomery is not NULL, the arrow becomes is
added to its children list.
*/
	MesomeryArrow (Mesomery* mesomery);
/*!
The destructor.
*/
	virtual ~MesomeryArrow ();
	
/*!
@param xml the xmlDoc used to save the document.

Used to save the arrow to the xmlDoc.
@return the xmlNode containing the serialized arrow.
*/
	xmlNodePtr Save (xmlDocPtr xml) const;
/*!
@param node: a pointer to the xmlNode containing the serialized arrow.

Used to load an arrow in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param w the GtkWidget inside which the arrow will be displayed.

Used to add a representation of the arrow in the widget.
*/
	void Add (GtkWidget* w) const;
/*!
@param w the GtkWidget inside which the arrow is displayed.

Used to update the representation of the arrow in the widget.
*/
	void Update (GtkWidget* w) const;
/*!
@param mesomer a mesomer

Sets \a mesomer at first end of the arrow. This does not affect coordinates,
alignment is dealt with elsewhere.
*/
	void SetStartMesomer (Mesomer *mesomer) {m_Start = mesomer;}
/*!
@return the mesomer at first end of the arrow.
*/
	Mesomer* GetStartMesomer () {return m_Start;}
/*!
@param mesomer a mesomer

Sets \a mesomer at last end of the arrow. This does not affect coordinates,
alignment is dealt with elsewhere.
*/
	void SetEndMesomer (Mesomer *mesomer) {m_End = mesomer;}
/*!
@return the mesomer at last end of the arrow.
*/
	Mesomer* GetEndMesomer () {return m_End;}
/*!
Exchange both ends or the arrow and their associated mesomers.
*/
	void Reverse ();

private:
	Mesomer *m_Start, *m_End;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_MESOMERY_ARROW_H
