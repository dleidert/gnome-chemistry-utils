// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/group.h
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_GROUP_H
#define GCCV_GROUP_H

/*!\file*/

#include "item.h"
#include <list>

namespace gccv {

/*!
@brief Item with Item children.

A Group is an Item grouping several children Item instances in a std::list.
Children might also be Group items themselves so that it allows for a
hierarchical tree if Item instances. The Canvas class owns a top level Group
(see Canvas::GetRoot()) and all Item instances in the canvas are descendants
of this root Group.
The Group class owns a pair of coordinates, x and y, which are used to shift all
the children.
*/
class Group: public Item
{
public:
/*!
@param canvas a Canvas.

Creates a new Group and sets it as a child of the root Group of \a canvas.
*/
	Group (Canvas *canvas);
/*!
@param canvas a Canvas.
@param x the horizontal group shift.
@param y the vertical group shift.

Creates a new Group at (\a x, \a y) and sets it as a child of the root Group of \a canvas.
*/
	Group (Canvas *canvas, double x, double y);
/*!
@param parent the Group to which the new Group will be added.
@param client the ItemClient for the new Group if any.

Creates a new Group inside \a parent and sets \a client as its associated
ItemClient.
*/
	Group (Group *parent, ItemClient *client = NULL);
/*!
@param parent the Group to which the new Group will be added.
@param x the horizontal group shift.
@param y the vertical group shift.
@param client the ItemClient for the new Group if any.

Creates a new Group at (\a x, \a y) inside \a parent and sets \a client as its
associated ItemClient.
*/
	Group (Group *parent, double x, double y, ItemClient *client = NULL);
/*!
The destructor. When a Group is destroyed, all its children are destroyed too.
*/
	virtual ~Group();

/*!
@param item the new child.

Adds \a item to the children list so that it will be displayed first, and so
will appear under other overlapping children. To add an Item on top all other
children, use thius method and then Group::MoveToFront().
*/
	void AddChild (Item *item);
/*!
@param item to remove.

Removes \a item to the children list but does not destroys it. \a item will not
be displayed anymore unless it is added to a new Group.
*/
	void RemoveChild (Item *item);
/*!
@param item to move in the list.

Changes the Item position in the children list so that it is displayed last on
top of other overlapping children.
*/
	void MoveToFront (Item *item);
/*!
@param item to move in the list.

Changes the Item position in the children list so that it is displayed first
below other overlapping children.
*/
	void MoveToBack (Item *item);

/*!
@param it a list iterator.

@return the first child Item. Actually, the one displayed first.
*/
	Item *GetFirstChild (std::list<Item *>::iterator &it);
/*!
@param it a list iterator initalized by a call to GetFirstChild().

@return the next child Item if any.
*/
	Item *GetNextChild (std::list<Item *>::iterator &it);

/*!
@param x0 the top left horizontal bound to adjust.
@param y0 the top left vertical bound to adjust.
@param x1 the bottom right horizontal bound to adjust.
@param y1 the bottom right top left vertical to adjustw.

Adjusts the parameters according to the shift values. This allows to evaluate
the absolute position of an Item inside the Canvas.
*/
	void AdjustBounds (double &x0, double &y0, double &x1, double &y1) const;
/*!
@param x the horizontal position
@param y the vertical position

Sets the Group shift values.
*/
	void SetPosition (double x, double y);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the nearest Item.

Implementation of Item::Distance() for the Group class. Sets \a item to the
descendant Item nearest to the given position.
*/
	double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.
@param x0 the top left horizontal bound of the region to draw.
@param y0 the top left vertical bound of the region to draw.
@param x1 the bottom right horizontal bound of the region to draw.
@param y1 the bottom right top left vertical bound of the region to draw.
@param is_vector whether the cairo_t is a vectorial context.

Draws Group children to \a cr, limiting things to the given region.
*/
	bool Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Group and hence all its descendants by changing the Group shift values.
*/
	void Move (double x, double y);

protected:
	void UpdateBounds ();

private:
	std::list<Item *> m_Children;
	double m_x, m_y;	// translation offset
};

}

#endif	//	 GCCV_GROUP_H
