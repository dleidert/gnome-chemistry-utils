/*
 * Gnome Chemistry Utils
 * gcugtk/cmd-context-gtk.h
 *
 * Copyright (C) 2010-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_CMD_CONTEXT_GTK_H
#define GCU_GTK_CMD_CONTEXT_GTK_H

#include <gcu/cmd-context.h>

/*!\file*/
namespace gcugtk {

/*!\class CmdContextGtk gcugtk/cmd-context-gtk.h
@brief A Gtk+ using gcu::CmdContext implementation.

This class uses GtkMessageBox dialogs to interact with the user.
*/
class CmdContextGtk: public gcu::CmdContext {
public:
/*!
@param App the application owning the command context.

The constructor.
*/
	CmdContextGtk (Application *App);

/*!
The destructor.
*/
	virtual ~CmdContextGtk ();

/*!
@param message the message string.
@param responses the responses that are meaningful such as 'ResponseYes | ResponseNo'.

Ask a question to the user about what should be done. A message box is displayed
showing the message and with a button for each bit in the \a response bits field.
@return the response chosen by the user.
*/
	Response GetResponse (char const *message, int responses);
/*!
@param message the message string.
@param severity the Severity of the situation.
@param modal whether the message should block the program execution or not.

Inform the user of an abnormal situation using a message box with an icon
corresponding to \a severity and a 'Close' button.
This method is pure virtual and must be implemented in derived classes.
*/
	void Message (char const *message, Severity severity, bool modal);
};

}	//	namespace gcugtk

#endif	/* GCU_GTK_CMD_CONTEXT_GTK_H */
