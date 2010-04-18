// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/client.h 
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

#ifndef GCCV_CLIENT_H
#define GCCV_CLIENT_H

#include <gcu/macros.h>

/*!\file*/

namespace gccv {

class Canvas;
class ItemClient;

/*!
@brief The document associated with the canvas.

The Client object might represent the document or its view. This class is used
to propagate mouse events to the document.
*/
class Client {
friend class Canvas;
public:
/*!
The standard constructor.
*/
	Client ();
/*!
The destructor.
*/
	virtual ~Client ();

	// Signals
/*!
@param client the ItemClient for the Item at the event position if any.
@param button the mouse button.
@param x horizontal event position.
@param y vertical event position.
@param state the GdkModifierType value for the event.

Called when a mouse button is pressed. \a client is NULL when there is no Item
at the event position, or the Item does not have an associated ItemClient.
@return true to stop other handlers from being invoked for the event. false
to propagate the event further.
*/
	virtual bool OnButtonPressed (ItemClient *client, unsigned button, double x, double y, unsigned state);
/*!
@param client the ItemClient for the Item at the event position if any.
@param button the mouse button.
@param x horizontal event position.
@param y vertical event position.
@param state the GdkModifierType value for the event.

Called when a mouse button is released.
\a client is NULL when there is no Item
at the event position, or the Item does not have an associated ItemClient.
@return true to stop other handlers from being invoked for the event. false
to propagate the event further.
*/
	virtual bool OnButtonReleased (ItemClient *client, unsigned button, double x, double y, unsigned state);
/*!
@param client the ItemClient for the Item at the event position if any.
@param x horizontal event position.
@param y vertical event position.
@param state the GdkModifierType value for the event.

Called when the mouse moves over the canvas, with no button pressed.
\a client is NULL when there is no Item
at the event position, or the Item does not have an associated ItemClient.
@@return true to stop other handlers from being invoked for the event. false
to propagate the event further.
*/
	virtual bool OnMotion (ItemClient *client, double x, double y, unsigned state);
/*!
@param client the ItemClient for the Item at the event position if any.
@param x horizontal event position.
@param y vertical event position.
@param state the GdkModifierType value for the event.

Called when the mouse moves over the canvas, with at least one button pressed.
\a client is NULL when there is no Item
at the event position, or the Item does not have an associated ItemClient.
@return true to stop other handlers from being invoked for the event. false
to propagate the event further.
*/
	virtual bool OnDrag (ItemClient *client, double x, double y, unsigned state);
/*!
@param state the GdkModifierType value for the event.

Called when the mouse leaves the Canvas.
@return true to stop other handlers from being invoked for the event. false
to propagate the event further.
*/
	virtual bool OnLeaveNotify (unsigned state);

/*!\var m_Canvas
The associated canvas.
*/
/*!\fn GetCanvas()
@return the associated canvas.
*/
GCU_PROT_PROP (Canvas *, Canvas)
};

}

#endif	//	GCCV_CLIENT_H
