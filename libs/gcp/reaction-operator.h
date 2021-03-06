// -*- C++ -*-

/*
 * GChemPaint library
 * reaction-operator.h
 *
 * Copyright (C) 2004-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_REACTION_OPERATOR_H
#define GCHEMPAINT_REACTION_OPERATOR_H

#include <gccv/item-client.h>
#include <gcu/object.h>

/*!\file*/
namespace gcp {

/*!\class ReactionOperator gcp/reaction-operator.h
\brief Class for '+' signs in chemical reaction equations.

Objects of this class are added when useful by the framework. There is no need
to create them manually.
*/
class ReactionOperator: public gcu::Object, public gccv::ItemClient
{
public:
/*!
The constructor. Adds a '+' sign in the chemical equation.
*/
	ReactionOperator ();
/*!
The destructor.
*/
	virtual ~ReactionOperator ();

/*!
Used to add a representation of the operator in the view.
*/
	void AddItem ();
/*!
@param x the x component of the transation vector.
@param y the y component of the transation vector.
@param z the z component of the transation vector (unused).

Moves the reaction operator.
*/
	virtual void Move (double x, double y, double z = 0);
/*!
@param state: the selection state of the operator.

Used to set the selection state of the operator.
The values of state might be gcp::SelStateUnselected, gcp::SelStateSelected,
gcp::SelStateUpdating, or gcp::SelStateErasing. Children will be selected too.
*/
	virtual void SetSelected (int state);
/*!
@param x the new x coordinate of the operator.
@param y the new y coordinate of the operator.

Sets the coordinates of the operator. The values are understood horizontally
as left side and vertically as base line.
*/
	void SetCoords (double x, double y);
/*!
@param x where to store the x coordinate of the operator.
@param y where to store the y coordinate of the operator.
@param z where to store the z coordinate of the operator or NULL for 2D representations.

Retrieves the current coordinates of the operator.
@return true if successful and false if an error occurs (if x or y is NULL).
*/
	bool GetCoords (double* x, double* y, double *z = NULL) const;
/*!
Used to retrieve the y coordinate for alignment.
@return y coordinate used for reaction operators alignment.
*/
	double GetYAlign () const;
/*!
@param property the property id as defined in objprops.h

Used when saving to get properties from a reaction operator. Currently only one
property is supported:
	GCU_PROP_POS2D.
*/
	std::string GetProperty (unsigned property) const;

/*!
@return the localized object generic name.
*/
	std::string Name ();

private:
	double m_x, m_y;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_OPERATOR_H
