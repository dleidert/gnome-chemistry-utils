// -*- C++ -*-

/* 
 * GChemPaint library
 * arrow.h 
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

#ifndef GCHEMPAINT_ARROW_H
#define GCHEMPAINT_ARROW_H

#include <gcu/object.h>

/*!\file*/
namespace gcp {

/*!\enum ArrowTypes
Enumeration of the known reaction arrow types.
*/
enum ArrowTypes
{
/*!
Simple reaction arrow.
*/
	SimpleArrow,
/*!
Double reaction arrow for reversible reactions with half heads.
*/
	ReversibleArrow,
/*!
Double reaction arrow for reversible reactions with full heads.
*/
	FullReversibleArrow,
};

/*!\class Arrow gcp/arrow.h
*/
class Arrow: public gcu::Object
{
public:
/*!
@param Type an arrow type id.

Used to create an arrow of type Id. Should only be called from the constructor
of a derived class.
*/
	Arrow(gcu::TypeId Type);
/*!
The destructor.
*/
	virtual ~Arrow();

/*!
@param node: a pointer to the xmlNode containing the serialized arrow.

Used to load an Arrow in memory.
This method must be called from derived classes overloaded Load methods.

@return true on succes, false otherwise.
*/
	bool Load (xmlNodePtr node);
/*!
@param w: the GtkWidget inside which the Object is displayed.
@param state: the selection state of the arrow.

Used to set the selection state of the arrow inside the widget.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing.
*/
	void SetSelected (GtkWidget* w, int state);
/*!
@param xstart the x coordinate of the start point.
@param ystart the y coordinate of the start point.
@param xend the x coordinate of the end point.
@param yend the y coordinate of the end point.

Sets the position and length of an arrow.
*/
	void SetCoords (double xstart, double ystart, double xend, double yend);
/*!
@param xstart where to store the x coordinate of the start point.
@param ystart where to store the y coordinate of the start point.
@param xend where to store the x coordinate of the end point.
@param yend where to store the y coordinate of the end point.

Retrieves the position of the arrow.
*/
	bool GetCoords (double* xstart, double* ystart, double* xend, double* yend);
/*!
@param x: the x component of the transation vector.
@param y: the y component of the transation vector.
@param z: the z component of the transation vector (unused).

Used to move an arrow. The third parameter is not taken into account.
*/
	void Move (double x, double y, double z = 0);
/*!
@param m: the Matrix2D of the transformation.
@param x: the x component of the center of the transformation.
@param y: the y component of the center of the transformation.

Used to move and/or transform an arrow.
*/
	void Transform2D (gcu::Matrix2D& m, double x, double y);
/*!
Used to retrieve the y coordinate for alignment. 
@return y coordinate used for arrows alignment.
*/
	double GetYAlign ();
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set common properties to arrows. Only one property is
currently supported: gcu::GCU_PROP_ARROW_COORDS.
@return true if the property could be set, or if the property is not relevant,
false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

protected:
/*!
@param xml: the xmlDoc used to save the document.
@param node: the node representing the Object.

This method must be called from derived classes overloaded Save methods.
@return true on succes, false otherwise.
*/
	bool Save (xmlDocPtr xml, xmlNodePtr node) const;
	
protected:
/*!
The x coordinate to the start point.
*/
	double m_x;
/*!
The y coordinate to the start point.
*/
	double m_y;
/*!
The x coordinate to the end point.
*/
	double m_width;
/*!
The y coordinate to the end point.
*/
	double m_height;
};

}	//	namespace gcp

#endif	//GCHEMPAINT_ARROW_H
