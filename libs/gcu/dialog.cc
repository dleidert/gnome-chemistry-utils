// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog.cc 
 *
 * Copyright (C) 2001-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "dialog.h"
#include "application.h"
#include "message.h"
#include <glib/gi18n-lib.h>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>

namespace gcu
{

static void on_OK (G_GNUC_UNUSED GtkWidget *widget, Dialog* pBox)
{
	if (pBox->Apply ())
		pBox->Destroy ();
}

static void on_apply (G_GNUC_UNUSED GtkWidget *widget, Dialog* pBox)
{
	pBox->Apply ();
}

static void on_cancel (G_GNUC_UNUSED GtkWidget *widget, Dialog* pBox)
{
	pBox->Destroy ();
}

static void on_help(G_GNUC_UNUSED GtkWidget *widget, Dialog* pBox)
{
	pBox->Help();
}

static bool on_destroy (G_GNUC_UNUSED GtkWidget *widget, Dialog* pBox)
{
	delete pBox;
	return true;
}

Dialog::Dialog (Application* App, char const *filename, const char* windowname, char const *domainname, DialogOwner *owner, void (*extra_destroy)(gpointer), gpointer data) throw (std::runtime_error):
	UIBuilder (filename, domainname)
{
	m_App = App;
	m_Owner = owner;
	if (owner && !owner->AddDialog (windowname, this)) {
		m_extra_destroy = NULL;
		// do we need to set m_Owner to NULL?
		throw std::runtime_error (_("Could not reference the new dialog."));
	}
	dialog = GTK_WINDOW (GetWidget (windowname));
	m_extra_destroy = extra_destroy;
	m_windowname = windowname;
	m_data = data;
	gtk_window_set_icon_name (dialog, App->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (on_destroy), this);
	GtkWidget* button = GetWidget ("OK");
	if (button)
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_OK), this);
	button = GetWidget ("apply");
	if (button)
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_apply), this);
	button = GetWidget ("cancel");
	if (button)
		g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_cancel), this);
	button = GetWidget ("help");
	if (button) {
		if (App->HasHelp ())
			g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_help), this);
		else
			gtk_widget_hide (button);
	}
}

Dialog::~Dialog()
{
	if (m_Owner)
		m_Owner->RemoveDialog (m_windowname);
}

void Dialog::Destroy()
{
	if (m_extra_destroy) m_extra_destroy (m_data);
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

bool Dialog::Apply ()
{
	return true;
}

void Dialog::Help ()
{
	m_App->OnHelp (m_windowname);
}

bool Dialog::GetNumber (GtkEntry *Entry, double *x, CheckType c, double min, double max)
{
	const gchar* text = gtk_entry_get_text (Entry);
	char *end;
	*x = strtod (text, &end);
	if (end != text + strlen (text)) {
		gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
		Message *box = new Message (m_App, _("Type a number"), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
		box->Run ();
		return false;
	}
	switch (c) {
	case MinEqMax:
		if ((*x < min) || (*x >= max)) {
			std::ostringstream str;
			str << _("Type a number greater than or equal to ") << min << _(" and lower than ") << max;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case MinMaxEq:
		if ((*x <= min) || (*x > max)) {
			std::ostringstream str;
			str << _("Type a number greater than ") << min << _(" and lower than or equal to ") << max;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case MinEqMaxEq:
		if ((*x < min) || (*x > max)) {
			std::ostringstream str;
			str << _("Type a number between ") << min << _(" and ") << max << _(", the limits are valid.");
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case MinMax:
		if ((*x <= min) || (*x >= max)) {
			std::ostringstream str;
			str << _("Type a number greater than ") << min << _(" and lower than ") << max;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case Max:
		if (*x >= max) {
			std::ostringstream str;
			str << _("Type a number lower than ") << max;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case Min:
		if (*x <= min) {
			std::ostringstream str;
			str << _("Type a number greater than ") << min;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case MaxEq:
		if (*x > max) {
			std::ostringstream str;
			str << _("Type a number lower than or equal to ") << max;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	case MinEq:
		if (*x < min) {
			std::ostringstream str;
			str << _("Type a number greater than or equal to ") << min;
			gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (Entry));
			Message *box = new Message (m_App, str.str ().c_str (), GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, GTK_WINDOW (dialog));
			box->Run ();
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}

void Dialog::SetTransientFor (GtkWindow *window)
{
	gtk_window_set_transient_for (dialog, window);
}

void Dialog::SetRealName (char const *name, DialogOwner *owner) throw (std::runtime_error)
{
	if (m_Owner)
		m_Owner->RemoveDialog (m_windowname);
	if (owner)
		m_Owner = owner;
	m_windowname = name;
	if (m_Owner && !m_Owner->AddDialog (name, this)) {
		m_extra_destroy = NULL;
		// do we need to set m_Owner to NULL?
		throw std::runtime_error (_("Could not reference the new dialog."));
	}
}

}	//	namespace gcu
