// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/message.h
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_MESSAGE_H
#define GCU_GTK_MESSAGE_H

#include <gtk/gtk.h>
#include <string>

/*!\file*/
namespace gcugtk {

class Application;

/*!\class Message gcugtk/message.h
@brief Message box.

This class implements a wrapper around GtkMessageDialog.
*/
	class Message {
friend class MessagePrivate;
public:
/*!
@param app the Application owning the message box.
@param message the text displayed inside the message box.
@param type the message box type.
@param buttons the buttons to display.
@param parent the parent window.
@param modal whether the message should be a modal dialog.

Contructs a new message box.
*/
	Message (Application *app, std::string &message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent = NULL, bool modal = false);
/*!
@param app the Application owning the message box.
@param message the text displayed inside the message box.
@param type the message box type.
@param buttons the buttons to display.
@param parent the parent window.
@param modal whether the message should be a modal dialog.

Contructs a new message box.
*/
	Message (Application *app, char const *message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent = NULL, bool modal = false);
/*!
The destructor.
*/
	virtual ~Message ();

/*!
Runs the modal message box.
@return the response from the dialog.
*/
	int Run ();
/*!
Displays a non modal message box.
*/
	void Show ();

private:
	GtkDialog *m_Window;
	unsigned m_delete_sgn, m_destroy_sgn, m_response_sgn;
};

}

#endif	//	GCU_GTK_MESSAGE_H
