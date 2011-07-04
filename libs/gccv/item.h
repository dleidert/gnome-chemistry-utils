// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/item.h
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

#ifndef GCCV_ITEM_H
#define GCCV_ITEM_H

#include <gcu/macros.h>
#include <cairo.h>

/*!\file */
/*!\def GCCV_ITEM_PROP()
Defines a private member with appropriate get/set methods. This macro should
be used when a property change needs a redraw of the item.
GCCV_ITEM_PROP((Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and three public methods:
\code
	void SetFoo(Type val);
	Type GetFoo();
\endcode

Calling SetFoo(val) will set the member and invalidate the item.
*/
#define GCCV_ITEM_PROP(type,member) \
public:	\
	void Set##member (type val) {	\
		m_##member = val;	\
		Invalidate ();	\
	}	\
	type Get##member (void) const {return m_##member;}	\
private:	\
	type m_##member;

/*!\def GCCV_ITEM_POS_PROP()
Defines a private member with appropriate get/set methods. This macro should
be used when a property change might change the bounds of the item.
GCCV_ITEM_POS_PROP((Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and three public methods:
\code
	void SetFoo(Type val);
	Type GetFoo();
\endcode

Calling SetFoo(val) will set the member and invalidate the item bounds.
*/
#define GCCV_ITEM_POS_PROP(type,member) \
public:	\
	void Set##member (type val) {	\
		Invalidate ();	\
		m_##member = val;	\
		BoundsChanged ();	\
		Invalidate ();	\
	}	\
	type Get##member (void) const {return m_##member;}	\
private:	\
	type m_##member;

namespace gccv {

class Canvas;
class Group;
class ItemClient;

/*!
@brief The base class for the canvas contents.

The Item class is the base class for everything that might be included inside
the Canvas associated widget. If the item represents an object, it can be
linked to it if the object derives from the ItemClient class. In that case, the
client object is notified when an event occurs for the item.
*/
class Item
{
public:
/*!
@param canvas a Canvas.

Creates a new Item and sets it as a child of the root Group of \a canvas.
*/
	Item (Canvas *canvas);
/*!
@param parent the Group to which the new Item will be added.
@param client the ItemClient for the new Item if any.

Creates a new Item inside \a parent and sets \a client as its associated ItemClient.
*/
	Item (Group *parent, ItemClient *client = NULL);
/*!
The destructor.
*/
	virtual ~Item ();

/*!
@param x0 where to store the top left horizontal bound.
@param y0 where to store the top left vertical bound.
@param x1 where to store the bottom right horizontal bound.
@param y1 where to store the bottom right vertical bound.

Retrieves the current bounds coordinate for the Item.
*/
	void GetBounds (double &x0, double &y0, double &x1, double &y1) const;
/*!
Invalidates the Item and force a redraw of the rectangular region defined by
its bounds.
*/
	void Invalidate () const;
/*!
@param visible whether the Item should be visible.

Show or Hide the Item according to the value of \a visible.
*/
	void SetVisible (bool visible);

	// virtual methods
/*!
@param x horizontal position
@param y vertical position
@param item where to store the Item.

Evaluates an approximative distance between the point defined by (@a x,\a y) and
the Item. A complex Item like a Group should set the nearest Item in \a item.
Simple item just set themselves.
\a return 0. when the point is inside the item, a reasonable small value when
the point is near the Item, and a large value when far. Defult implementation
returns G_MAXDOUBLE.
*/
	virtual double Distance (double x, double y, Item **item) const;
/*!
@param cr a cairo_t.

Builds the cairo path corresponding to the Item bounds. Doesn't draw anything.
*/
	virtual void BuildPath (cairo_t *cr) const;
/*!
@param cr a cairo_t.
@param is_vector whether the cairo_t is a vectorial context.

Draws Item to \a cr. \a is_vector might be used to enhance rendering in the case
of a raster target.
Derived classes should override at least one of the Draw() methods
*/
	virtual void Draw (cairo_t *cr, bool is_vector) const;
/*!
@param cr a cairo_t.
@param x0 the top left horizontal bound of the region to draw.
@param y0 the top left vertical bound of the region to draw.
@param x1 the bottom right horizontal bound of the region to draw.
@param y1 the bottom right top left vertical bound of the region to draw.
@param is_vector whether the cairo_t is a vectorial context.

Draws Item to \a cr, limiting the operations to the given rectangular region.
\a is_vector might be used to enhance rendering in the case
of a raster target.
Derived classes should override at least one of the Draw() methods
@return true if done. Default implementation returns false. When false is
returned the other Draw() method is called.
*/
	virtual bool Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const;
/*!
@param x the horizontal deplacement
@param y the vertical deplacement

Moves the Item.
*/
	virtual void Move (double x, double y);
/*!
@return whether the item parent is the canvas root item.
*/
	bool IsTopLevel () const;

protected:
/*!
Must be called when the bounds have changed.
*/
	void BoundsChanged ();
/*!
Updates Item::m_x0, Item::m_y0, Item::m_x1 and Item::m_y1. All derived classes
should implement this method to set the bounds and call Item::UpdateBounds()
when done.
*/
	virtual void UpdateBounds ();
/*!
@return the Canvas enclosing the Item.

*/
	Canvas const *GetCanvas () const {return m_Canvas;}

protected:
/*!
The top left horizontal bound.
*/
	double m_x0;
/*!
The top left vertical bound.
*/
	double m_y0;
/*!
The bottom right horizontal bound.
*/
	double m_x1;
/*!
The bottom right vertical bound.
*/
	double m_y1;

private:
	Canvas *m_Canvas;
	bool m_CachedBounds;
	bool m_NeedsRedraw;

/*!\fn  SetClient(ItemClient *Client)
@param Client an ItemClient instance.

Sets the Item Client associated with the Item.
*/
/*!\fn  GetClient()
@return the ItemClient associated with the Item.
*/
GCU_POINTER_PROP (ItemClient, Client)
/*!\fn GetParent()
@return the Item parent Group.
*/
GCU_RO_POINTER_PROP (Group, Parent)
/*!\fn GetVisible()
@return true if the item is visible, false if hidden.
*/
GCU_RO_PROP (bool, Visible)
/*!\fn SetOperator(cairo_operator_t Operator)
@param Operator a cairo_operator_t.

Sets the cairo_operator_t used by the item.
*/
/*!\fn GetOperator()
@return the cairo_operator_t used when rendering the item.
*/
GCCV_ITEM_PROP (cairo_operator_t, Operator);
};

}

#endif	//	 GCCV_ITEM_H
