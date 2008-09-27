// -*- C++ -*-

/* 
 * GChemPaint library
 * mesomery.h 
 *
 * Copyright (C) 2002-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_MESOMERY_H
#define GCHEMPAINT_MESOMERY_H

#include <gcu/object.h>

namespace gcp {

class Mesomer;

/*!\file*/
/*!\class Mesomery gcp/mesomery.h
Represents mesomery relationships.*/
class Mesomery: public gcu::Object
{
public:
/*!
The default constructor. Builds a new empty mesomery relationship.
*/
	Mesomery ();
	virtual ~Mesomery ();

/*!
@param node: a pointer to the xmlNode containing the serialized arrow.

Used to load an arrow in memory.
@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param Children the molecules and arrow involved in the mesomery.

Tries to associate mesomers and arrows so that the relationship is clearly
established and aligns the objects. On failure, it throws an std::invalid_argument
exception.
@return true on success.
*/
	bool Build (std::list<gcu::Object*>& Children) throw (std::invalid_argument);
/*!
@param m the Matrix2D of the transformation.
@param x the x component of the center of the transformation.
@param y the y component of the center of the transformation.

Mesomeries can't be currently rotated. This method don't do anything. It is
just there to inhibit the default behavior.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
@param UIManager the GtkUIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

Builds the contextual menu for the mesomery.
@return true (failure would be a bug).
*/
	bool BuildContextualMenu (GtkUIManager *UIManager, gcu::Object *object, double x, double y);
/*!
@param Signal the appropriate SignalId
@param Child the child which emitted the signal or NULL

This function is called by the framework one of the molecules or arrows has changed.
Realigns if the mesomery is still valid or destroys it.

@return true to propagate the signal to the parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
@param split whether to split the mesomery when arrows are missing.

Checks the validity of the mesomery, at least that each arrow is associated with
two mesomers and all mesomers are related by arrows. Currently it does not check
that the molecules are really mesomeric, but this might be implemented in the
future.
@return true if the mesomery is valid or could be splitted, false otherwise.
*/
	bool Validate (bool split);
/*!
Aligns the children.
*/
	void Align ();
/*!
@return the alignment ordinate of the top mesomer.
*/
	double GetYAlign ();

private:
	Mesomery (gcu::Object* parent, Mesomer *mesomer);
};

}	//	namespace gcp

#endif	//GCHEMPAINT_MESOMERY_H
