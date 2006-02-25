// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * filechooser.cc 
 *
 * Copyright (C) 2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <config.h>
#include "filechooser.h"
#include <gcu/application.h>
#include <gcu/document.h>
#include <glib/gi18n.h>

using namespace gcu;

FileChooser::FileChooser (Application *App, bool Save,list<char const*> mime_types, Document *pDoc, char const *title)
{
	char* filename;
	m_pDoc = pDoc;
	dialog = (GtkFileChooser*) gtk_file_chooser_dialog_new (
															(title != NULL)? title: ((Save)? _("Save as"): _("Open")),
															App->GetWindow(),
															(Save)? GTK_FILE_CHOOSER_ACTION_SAVE: GTK_FILE_CHOOSER_ACTION_OPEN,
															(Save)? GTK_STOCK_SAVE: GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
															NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
	GtkFileFilter* filter = gtk_file_filter_new ();
	list<char const*>::iterator i, iend = mime_types.end ();
	for (i = mime_types.begin (); i != iend; i++)
		gtk_file_filter_add_mime_type (filter, *i);
	if (!Save)
		gtk_file_chooser_set_select_multiple (chooser, true);
	// Now add network directories
	gtk_file_chooser_set_local_only (chooser, false);
	char const* dir = App->GetCurDir ();
	if (dir)
		gtk_file_chooser_set_current_folder_uri (chooser, dir);
	while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		if (Save) {
			filename = gtk_file_chooser_get_uri (chooser);
			if (!App->FileProcess (filename, Save, GTK_WINDOW (dialog), m_pDoc)) {
					g_free (filename);
					break;
				}
		} else {
			GSList* files = gtk_file_chooser_get_uris (chooser);
			GSList* iter = files;
			while (iter) {
				filename = (char*) iter->data;
				if (!App->FileProcess(filename, Save, GTK_WINDOW (dialog), m_pDoc)) {
					g_free (filename);
					break;
				}
				g_free (filename);
				iter = iter->next;
			}
			g_slist_free (files);
			break;
		}
	}
	gtk_widget_destroy (GTK_WIDGET (dialog));
}
