// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-separator.h
 *
 * Copyright (C) 2013 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#ifndef GCHEMPAINT_REACTION_SEPARATOR_H
#define GCHEMPAINT_REACTION_SEPARATOR_H

#include <gccv/item-client.h>
#include <gcu/object.h>

/*!\file*/
namespace gcp {

/*!\class ReactionOperator gcp/reaction-separator.h
\brief Class for ", " strings used to separate objects attached to an arrow.

Objects of this class are added when useful by the framework. There is no need
to create them manually.
*/
class ReactionSeparator: public gcu::Object, public gccv::ItemClient
{
public:
/*!
The constructor. Adds a ", " string to separate objects attached to an arrow.
*/
	ReactionSeparator ();
/*!
The destructor.
*/
	virtual ~ReactionSeparator ();

/*!
Used to add a representation of the separator in the view.
*/
	void AddItem ();
/*!
@param x the x component of the transation vector.
@param y the y component of the transation vector.
@param z the z component of the transation vector (unused).

Moves the reaction separator.
*/
	virtual void Move (double x, double y, double z = 0);
/*!
@param state: the selection state of the separator.

Used to set the selection state of the separator.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing. Children will be selected too.
*/
	virtual void SetSelected (int state);
/*!
@param x the new x coordinate of the separator.
@param y the new y coordinate of the separator.

Sets the coordinates of the separator. The values are understood horizontally
as left side and vertically as base line.
*/
	void SetCoords (double x, double y);
/*!
@param x where to store the x coordinate of the separator.
@param y where to store the y coordinate of the separator.
@param z where to store the z coordinate of the separator or NULL for 2D representations.

Retrieves the current coordinates of the separator.
@return true if successful and false if an error occurs (if x or y is NULL).
*/
	bool GetCoords (double* x, double* y, double *z = NULL) const;
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate used for reaction separators alignment.
*/
	virtual double GetYAlign ();

/*!
@return the localized object generic name.
*/
	std::string Name ();

private:
	double m_x, m_y;
	PangoLayout *m_Layout;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_SEPARATOR_H
