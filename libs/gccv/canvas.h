// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gccv/canvas.h
 *
 * Copyright (C) 2008-2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_CANVAS_H
#define GCCV_CANVAS_H

#include <gcu/macros.h>
#include <gtk/gtk.h>

/*!\file*/
/*!\namespace gccv
\brief Gnome Chemistry Canvas library namespace

The namespace used for the canvas C++ classes used by GChemPaint.
*/

namespace gccv {

class Client;
class Group;
class Item;

/*!\class Canvas gccv/canvas.h
\brief the Canvas class

This class is the container class for the items and is the only one interacting
with the underlying Gtk+ framework. It uses a private widget derived from
GtkDrawingArea.
*/

class CanvasPrivate;
class Canvas
{
friend class CanvasPrivate;
public:
/*!
@param client the gccv::Client for the canvas or NULL.

Constructs a canvas for \a client which is mandatory if some interaction
with the canvas is needed.
*/
	Canvas (Client *client);
/*!
The destructor.
You should not delete the canvas, as it will be destroyed with
the enclosing widget. Instead, if you never show the widget in a window, use:
\code
	gtk_widget_destroy (canvas->GetWidget ());
\endcode

Items are stored in an ordered tree whose top node is a gccv::Group instance
which can be accessed by the GetRoot() method.
*/
	virtual ~Canvas();

/*!
@param x the x coordiante.
@param y the y coordinate.

Used to get the top item at or near the given position.
@return the found item or NULL if no item is nearer than the current gap
from the given position.
*/
	Item *GetItemAt (double x, double y);
/*!
@return the widget used by the canvas.
*/
	GtkWidget *GetWidget () {return m_Widget;}
/*!
@param x0 the x coordinate for the top left of the invalidated rectangle.
@param y0 the y coordinate for the top left of the scrolling rectangle.
@param x1 the x coordinate for the bottom right of the scrolling rectangle.
@param y1 the y coordinate for the bottom right of the scrolling rectangle.

*/
	void Invalidate (double x0, double y0, double x1, double y1);
/*!
@param color a GOColor.

Sets the background color for the canvas widget
*/
	void SetBackgroundColor (GOColor color);
/*!
@param zoom the new zoom level.

Sets the zoom level for the canvas.
*/
	void SetZoom (double zoom);
/*!
@param cr a cairo context.
@param is_vector whether the cairo context is vectorial or raster.

Renders the current canvas to the cairo context.
*/
	void Render (cairo_t *cr, bool is_vector);

private:
	GtkWidget *m_Widget;
	Client *m_Client;
	bool m_Dragging;

/*!\fn GetZoom()
@return the current zoom level for the canvas.
*/
GCU_RO_PROP (double, Zoom)
/*!\fn GetRoot()
@return the root item.
*/
GCU_RO_PROP (Group *, Root)
/*!\fn SetGap(double gap)
@param gap a distance at which an item might be from an event location
to be selected for the event.

Sets the maximum distance at which an item might be from an event location
to be selected for the event. When an even as a mouse button click occurs,
and if it occurs on an item, the appropriate method of the item client will
be called; else if it occurs on the background, the nearest item will be used
if its distance from the event location is less than \a gap. If no item can
be used, the event will have no associated gccv::ItemClient instance.
*/
/*!\fn GetGap()
@return the current gap for the canvas.
*/
/*!\fn GetRefGap()
@return the current gap for the canvas as a reference.
*/
GCU_PROP (double, Gap)
/*!\fn GetColor()
@return the foreground color.
*/
GCU_PROP (GOColor, Color)
/*!\fn GetFont()
@return the font description.
*/
GCU_RO_PROP (PangoFontDescription *, Font)
/*!\fn GetLastEventState()
@return the GdkModifierType value for the last received event.
*/
/*!\fn GetBackgroundColor()
@return the background color.
*/
GCU_RO_PROP (GOColor, BackgroundColor)
/*!\fn GetLastEventState()
@return the GdkModifierType value for the last received event.
*/
GCU_RO_PROP (unsigned, LastEventState)
};

}

#endif	//	 GCCV_CANVAS_H
