// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <glib/gi18n.h>
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

Dialog::Dialog (Application* App, const char* filename, const char* windowname, DialogOwner *owner, void (*extra_destroy)(gpointer), gpointer data)
{
	m_App = App;
	m_Owner = owner;
	if (owner && !owner->AddDialog (windowname, this)) {
		xml = NULL;
		dialog = NULL;
		m_extra_destroy = NULL;
		return;
	}
	m_Owner = owner;
	if (filename) {
		xml =  glade_xml_new (filename, windowname, NULL);
		m_extra_destroy = extra_destroy;
		m_windowname = windowname;
		m_data = data;
		if (xml)  glade_xml_signal_autoconnect (xml);
		dialog = GTK_WINDOW (glade_xml_get_widget(xml, windowname));
		gtk_window_set_icon_name (dialog, App->GetIconName ().c_str ());
		g_signal_connect (G_OBJECT (dialog), "destroy", G_CALLBACK (on_destroy), this);
		GtkWidget* button = glade_xml_get_widget (xml, "OK");
		if (button) g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_OK), this);
		button = glade_xml_get_widget (xml, "apply");
		if (button) g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_apply), this);
		button = glade_xml_get_widget (xml, "cancel");
		if (button) g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_cancel), this);
		button = glade_xml_get_widget(xml, "help");
		if (button) {
			if (App->HasHelp ())
				g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_help), this);
			else
				gtk_widget_hide (button);
		}
	} else {
		xml = NULL;
		dialog = NULL;
		m_extra_destroy = NULL;
		m_windowname = "";
	}
}

Dialog::~Dialog()
{
	if (xml)
		g_object_unref (G_OBJECT (xml));
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
		GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Type a number")));
		gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
		if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
			gtk_widget_destroy (GTK_WIDGET (box));
		return false;
	}
	switch (c) {
	case MinEqMax:
		if ((*x < min) || (*x >= max)) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number greater than or equal %g and lower than to %g"), min, max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case MinMaxEq:
		if ((*x <= min) || (*x > max)) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number greater than %g and lower than or equal to %g"), min, max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case MinEqMaxEq:
		if ((*x < min) || (*x > max)) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number between %g and %g, the limits are valid."), min, max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case MinMax:
		if ((*x <= min) || (*x >= max)) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number greater than %g and lower than %g"), min, max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case Max:
		if (*x >= max) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number lower than %g"), max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case Min:
		if (*x <= min) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number greater than %g"), min);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case MaxEq:
		if (*x > max) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number lower than or equal to %g"), max);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	case MinEq:
		if (*x < min) {
			snprintf (m_buf, sizeof (m_buf), _("Type a number greater than or equal to %g"), min);
			GtkDialog* box = GTK_DIALOG (gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_window_set_icon_name (GTK_WINDOW (box), m_App->GetName ().c_str ());
			if (gtk_dialog_run (box) != GTK_RESPONSE_NONE)
				gtk_widget_destroy (GTK_WIDGET (box));
			return false;
		}
		break;
	default:
		break;
	}
	return true;
}

}	//	namespace gcu
