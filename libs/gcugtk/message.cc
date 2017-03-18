// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/message.cc
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

#include "config.h"
#include "application.h"
#include "message.h"

namespace gcugtk {

class MessagePrivate {
public:
	static void Close (Message *message);
	static void Destroyed (Message *message);
};

void MessagePrivate::Close (Message *message)
{
	delete message;
}

void MessagePrivate::Destroyed (Message *message)
{
	gtk_widget_destroy (GTK_WIDGET (message->m_Window));
	message->m_Window = NULL;
}

Message::Message (Application *app, std::string &message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent, bool modal)
{
	GtkDialogFlags flags = static_cast <GtkDialogFlags> (((parent)? GTK_DIALOG_DESTROY_WITH_PARENT: 0) | ((modal)? GTK_DIALOG_MODAL: 0));
	m_Window = GTK_DIALOG (gtk_message_dialog_new (parent, flags, type, buttons, "%s", message.c_str ()));
	gtk_window_set_icon_name (GTK_WINDOW (m_Window), app->GetIconName ().c_str ());
	gtk_widget_show (GTK_WIDGET (m_Window));
	m_delete_sgn = g_signal_connect_swapped (G_OBJECT (m_Window), "delete-event", G_CALLBACK (MessagePrivate::Destroyed), this);
	m_destroy_sgn = g_signal_connect_swapped (G_OBJECT (m_Window), "destroy-event", G_CALLBACK (MessagePrivate::Destroyed), this);
	m_response_sgn = (modal)? 0: g_signal_connect_swapped (G_OBJECT (m_Window), "response", G_CALLBACK (MessagePrivate::Close), this);
}

Message::Message (Application *app, char const *message, GtkMessageType type, GtkButtonsType buttons, GtkWindow *parent, bool modal)
{
	GtkDialogFlags flags = static_cast <GtkDialogFlags> (((parent)? GTK_DIALOG_DESTROY_WITH_PARENT: 0) | ((modal)? GTK_DIALOG_MODAL: 0));
	m_Window = GTK_DIALOG (gtk_message_dialog_new (parent, flags, type, buttons, "%s", message));
	gtk_window_set_icon_name (GTK_WINDOW (m_Window), app->GetIconName ().c_str ());
	gtk_widget_show (GTK_WIDGET (m_Window));
	m_delete_sgn = g_signal_connect_swapped (G_OBJECT (m_Window), "delete-event", G_CALLBACK (MessagePrivate::Destroyed), this);
	m_destroy_sgn = g_signal_connect_swapped (G_OBJECT (m_Window), "destroy-event", G_CALLBACK (MessagePrivate::Destroyed), this);
	m_response_sgn = (modal)? 0: g_signal_connect_swapped (G_OBJECT (m_Window), "response", G_CALLBACK (MessagePrivate::Close), this);
}

Message::~Message ()
{
	if (m_Window) {
		g_signal_handler_disconnect (m_Window, m_delete_sgn);
		g_signal_handler_disconnect (m_Window, m_destroy_sgn);
		if (m_response_sgn)
			g_signal_handler_disconnect (m_Window, m_response_sgn);
		gtk_widget_destroy (GTK_WIDGET (m_Window));
		m_Window = NULL;
	}
}

int Message::Run ()
{
	if (m_response_sgn) {
		g_signal_handler_disconnect (m_Window, m_response_sgn);
		m_response_sgn = 0;
	}
	int res = gtk_dialog_run (m_Window);
	delete this;
	return res;
}

void Message::Show ()
{
	return gtk_widget_show_all (GTK_WIDGET (m_Window));
}

}	// namespace gcu