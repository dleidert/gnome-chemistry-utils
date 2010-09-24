/* 
 * Gnome Chemistry Utils
 * cmd-context-gtk.cc
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "cmd-context-gtk.h"
#include <gsf/gsf-impl-utils.h>
#include <stdio.h>

namespace gcu {

CmdContextGtk::CmdContextGtk (): CmdContext ()
{
	m_GOCmdContext = NULL;
}

CmdContextGtk::~CmdContextGtk ()
{
}

CmdContext::Response CmdContextGtk::GetResponse (Application *App, char const *message, int responses)
{
	int buttons = 0;
	if (responses & ResponseOK)
		buttons |= GTK_BUTTONS_OK;
	if (responses & ResponseCancel)
		buttons |= GTK_BUTTONS_CANCEL;
	if ((responses & (ResponseYes | ResponseNo)) == (ResponseYes | ResponseNo))
		buttons |= GTK_BUTTONS_YES_NO;
	if (responses & ResponseClose)
		buttons |= GTK_BUTTONS_CLOSE;
	GtkWidget *dlg = gtk_message_dialog_new_with_markup (App->GetWindow (),
	                                          static_cast <GtkDialogFlags> (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
	                                          GTK_MESSAGE_QUESTION,
	                                          static_cast <GtkButtonsType> (buttons),
	                                          message);
	gtk_window_set_icon_name(GTK_WINDOW (dlg), App->GetIconName ().c_str ());
	buttons = gtk_dialog_run (GTK_DIALOG (dlg));
	gtk_widget_destroy (dlg);
	switch (buttons) {
	case GTK_RESPONSE_OK:
		return ResponseOK;
	case GTK_RESPONSE_CANCEL:
		return ResponseCancel;
	case GTK_RESPONSE_YES:
		return ResponseYes;
	case GTK_RESPONSE_NO:
		return ResponseNo;
	case GTK_RESPONSE_CLOSE:
		return ResponseClose;
	default:
		return ResponseDefault;
	}
}

void CmdContextGtk::Message (Application *App, char const *message, Severity severity, bool modal)
{
	GtkMessageType type;
	switch (severity) {
	case SeverityMessage:
		type = GTK_MESSAGE_INFO;
		break;
	case SeverityWarning:
		type = GTK_MESSAGE_WARNING;
		break;
	case SeverityError:
		type = GTK_MESSAGE_ERROR;
		break;
	default:
		type = GTK_MESSAGE_OTHER;
		break;
	}
	GtkWidget *dlg = gtk_message_dialog_new_with_markup (App->GetWindow (),
	                                          static_cast <GtkDialogFlags> (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
	                                          type,
	                                          GTK_BUTTONS_CLOSE,
	                                          message);
	gtk_window_set_icon_name(GTK_WINDOW (dlg), App->GetIconName ().c_str ());
	g_signal_connect (G_OBJECT (dlg), "response", G_CALLBACK (gtk_widget_destroy), NULL);
	if (modal)
		gtk_dialog_run (GTK_DIALOG (dlg));
	else
		gtk_widget_show_all (dlg);
}

}	//	namespace gcu
