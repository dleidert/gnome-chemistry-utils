// -*- C++ -*-

/* 
 * Gnome Crystal
 * dialog.h 
 *
 * Copyright (C) 2001-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "dialog.h"
#include <string.h>
#include <glib/gi18n.h>

static void on_OK(GtkWidget *widget, gcDialog* dialog)
{
	dialog->Apply();
	dialog->Destroy();
}

static void on_apply(GtkWidget *widget, gcDialog* dialog)
{
	dialog->Apply();
}

static void on_cancel(GtkWidget *widget, gcDialog* dialog)
{
	dialog->Destroy();
}
	
static bool on_destroy(GtkWidget *widget, gcDialog* dialog)
{
	delete dialog;
	return true;
}

static void on_help(GtkWidget* widget, gcDialog* dialog)
{
	dialog->Help();
}

gcDialog::gcDialog(const char* filename, const char* windowname)
{
	xml =  glade_xml_new(filename, windowname, NULL);
	if (xml)  glade_xml_signal_autoconnect (xml);
	dialog = GTK_WINDOW(glade_xml_get_widget(xml, m_WindowName = windowname));
	g_signal_connect(G_OBJECT(dialog), "destroy", GTK_SIGNAL_FUNC(on_destroy), this);
	GtkWidget* button = glade_xml_get_widget(xml, "OK");
	if (button) g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(on_OK), this);
	button = glade_xml_get_widget(xml, "apply");
	if (button) g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(on_apply), this);
	button = glade_xml_get_widget(xml, "cancel");
	if (button) g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(on_cancel), this);
	button = glade_xml_get_widget(xml, "help");
	if (button) g_signal_connect(G_OBJECT(button), "clicked", GTK_SIGNAL_FUNC(on_help), this);
	notebook = (GtkNotebook*)glade_xml_get_widget(xml, "notebook");
}

gcDialog::~gcDialog()
{
}

void gcDialog::Destroy()
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

bool gcDialog::Apply() {}

void gcDialog::Help()
{
	gchar *tag = g_strdup_printf("%s-%s", PACKAGE, m_WindowName);
	
	gnome_help_display(PACKAGE, tag, NULL);
	g_free(tag);
}

bool gcDialog::GetNumber(GtkEntry *Entry, double *x, gcCheckType c, double min, double max)
{
	const gchar* text = gtk_entry_get_text (Entry);
	char *end;
	*x = strtod(text, &end);
	if (end != text + strlen(text))
	{
		gtk_window_set_focus(GTK_WINDOW(dialog), GTK_WIDGET(Entry));
		GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL,
										GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, _("Type a number")));
		gtk_dialog_run(box);
		return false;
	}
	switch (c)
	{
	case gccMinEqMax:
		if ((*x < min) || (*x >= max))
		{
			snprintf(m_buf, sizeof(m_buf), _("Type a number between %g and %g"), min, max);
			GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_dialog_run(box);
			return false;
		}
		break;
	case gccMinEqMaxEq:
		if ((*x < min) || (*x > max))
		{
			snprintf(m_buf, sizeof(m_buf), _("Type a number between %g and %g"), min, max);
			GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_dialog_run(box);
			return false;
		}
		break;
	case gccMinMax:
		if ((*x <= min) || (*x >= max))
		{
			snprintf(m_buf, sizeof(m_buf), _("Type a number between %g and %g"), min, max);
			GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_dialog_run(box);
			return false;
		}
		break;
	case gccMin:
		if (*x < min)
		{
			snprintf(m_buf, sizeof(m_buf), _("Type a number greater than %g"), min);
			GtkDialog* box = GTK_DIALOG(gtk_message_dialog_new(GTK_WINDOW(dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, m_buf));
			gtk_dialog_run(box);
			return false;
		}
		break;
	}
	return true;
}
