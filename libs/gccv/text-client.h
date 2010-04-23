// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gccv/text-client.h 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCCV_TEXT_CLIENT_H
#define GCCV_TEXT_CLIENT_H

/*!\file*/

#include <gccv/item-client.h>
#include <gtk/gtk.h>

namespace gccv {

/*!
@brief ItemClient for text items.

A specialized ItemClient class for Text items. This class is virtual.
*/
class TextClient: public ItemClient
{
public:
/*!
The default constructor.
*/
	TextClient ();
/*!
The destructor.
*/
	virtual ~TextClient ();

/*!
@param start new selection start index.
@param end new selection end index.

Notifies the client that the selection position has changed.
*/
	virtual void SelectionChanged (unsigned start, unsigned end) = 0;
/*!
@param pos the new cursor position.

Notifies the client that the text has been changed.
*/
	virtual void TextChanged (unsigned pos) = 0;
/*!
@param interline the new interline spacing.

Notifies the client that the interline has been changed.
*/
	virtual void InterlineChanged (G_GNUC_UNUSED double interline) {;}
/*!
@param justification the new justification.

Notifies the client that the justification has been changed.
*/
	virtual void JustificationChanged (G_GNUC_UNUSED GtkJustification justification) {;}

};

}

#endif	//	GCCV_CLIENT_H
