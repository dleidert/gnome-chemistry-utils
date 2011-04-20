// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcugtk/message.h 
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_MESSAGE_H
#define GCU_GTK_MESSAGE_H

#include <gtk/gtk.h>
#include <string>

namespace gcugtk {

class Application;

class Message {
friend class MessagePrivate;
public:
	Message (Application *app, std::string &message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent = NULL);
	Message (Application *app, char const *message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent = NULL);
	virtual ~Message ();

	int Run ();
	void Show ();

private:
	GtkDialog *m_Window;
	unsigned m_delete_sgn, m_destroy_sgn, m_response_sgn;
};

}

#endif	//	GCU_GTK_MESSAGE_H
