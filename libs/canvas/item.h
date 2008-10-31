// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * canvas/item.h 
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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
Defines a private member with appropriate get/set methods.
GCU_PROP((Type,Foo) expands to one private member:
\code
	Type m_Foo;
\endcode

and three public methods:
\code
	void SetFoo(Type val);
	Type GetFoo();
	Type& GetRefFoo();
\endcode

The last one allows code as:
\code
	obj.GetRefFoo() = val;
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
	type &GetRef##member (void) {return m_##member;}	\
private:	\
	type m_##member;

namespace gccv {

class Canvas;
class Group;
class ItemClient;

class Item
{
public:
	Item (Canvas *canvas);
	Item (Group *parent, ItemClient *client = NULL);
	virtual ~Item();

	void GetBounds (double &x0, double &y0, double &x1, double &y1) const;
	void Invalidate () const;
	void SetVisible (bool visible);

	// virtual methods
	virtual double Distance (double x, double y, Item **item) const;
	virtual void Draw (cairo_t *cr, bool is_vector) const;
	virtual bool Draw (cairo_t *cr, double x0, double y0, double x1, double y1, bool is_vector) const;
	virtual void Move (double x, double y);

protected:
	void BoundsChanged ();
	virtual void UpdateBounds ();

protected:
	double m_x0, m_y0, m_x1, m_y1;

private:
	Canvas *m_Canvas;
	bool m_CachedBounds;
	bool m_NeedsRedraw;

GCU_POINTER_PROP (ItemClient, Client)
GCU_POINTER_PROP (Group, Parent)
GCU_RO_PROP (bool, Visible)
};

}

#endif	//	 GCCV_ITEM_H
