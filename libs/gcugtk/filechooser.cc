// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/filechooser.cc
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcugtk/application.h>
#include <gcu/document.h>
#include <glib/gi18n-lib.h>

using namespace std;

namespace gcugtk
{

FileChooser::FileChooser (Application *App, bool Save, list<string> mime_types, gcu::Document *pDoc, char const *title, GtkWidget *extra_widget)
{
	char* filename = NULL;
	m_pDoc = pDoc;
	dialog = (GtkFileChooser*) gtk_file_chooser_dialog_new (
															(title != NULL)? title: ((Save)? _("Save as"): _("Open")),
															App->GetWindow(),
															(Save)? GTK_FILE_CHOOSER_ACTION_SAVE: GTK_FILE_CHOOSER_ACTION_OPEN,
															(Save)? GTK_STOCK_SAVE: GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
															NULL);
	if (extra_widget)
		gtk_file_chooser_set_extra_widget (dialog, extra_widget);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
	GtkFileFilter* filter = gtk_file_filter_new ();
	list<string>::iterator i, iend = mime_types.end ();
	for (i = mime_types.begin (); i != iend; i++)
		gtk_file_filter_add_mime_type (filter, (*i).c_str ());
	GtkComboBoxText *format_combo = NULL;
	if (!Save)
		gtk_file_chooser_set_select_multiple (chooser, true);
	if (mime_types.size () > 1) {
		GtkWidget *grid = gtk_grid_new ();
		if (gtk_check_version (3, 2, 0))
			gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
		else
			gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
		GtkWidget *label = gtk_label_new_with_mnemonic (_("File _type:"));
		format_combo = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
		gtk_combo_box_text_append_text (format_combo, _("Automatic"));
		for (i = mime_types.begin (); i != iend; i++) {
			char *type = go_mime_type_get_description ((*i).c_str ());
			if (type) {
				gtk_combo_box_text_append_text (format_combo, type);
				g_free (type);
			} else
				gtk_combo_box_text_append_text (format_combo, (*i).c_str ());
		}
		gtk_combo_box_set_active (GTK_COMBO_BOX (format_combo), 0);

		gtk_container_add (GTK_CONTAINER (grid), label);
		gtk_container_add (GTK_CONTAINER (grid), GTK_WIDGET (format_combo));
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (format_combo));

		if (extra_widget) {
			if (GTK_IS_CONTAINER (extra_widget))
				gtk_container_add (GTK_CONTAINER (extra_widget), grid);
			else
				g_warning ("not implemented, please file a bug report");
		} else
			gtk_file_chooser_set_extra_widget (dialog, grid);
		gtk_widget_show_all (grid);
	}
	gtk_file_chooser_set_filter (chooser, filter);
	// Now add network directories
	gtk_file_chooser_set_local_only (chooser, false);
	char const* dir = App->GetCurDir ();
	if (dir)
		gtk_file_chooser_set_current_folder_uri (chooser, dir);
	while (gtk_widget_show_all (GTK_WIDGET (dialog)), gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		// find the mime_type
		string mime_type;
		if (mime_types.size () == 1)
			mime_type = mime_types.front ();
		else if (mime_types.size () > 0) {
			int j = gtk_combo_box_get_active (GTK_COMBO_BOX (format_combo));
			if (j > 0) {
				i = mime_types.begin ();
				while (--j > 0)
					i++;
				mime_type = *i;
			}
		}
		if (Save) {
			filename = gtk_file_chooser_get_uri (chooser);
			if (mime_type.length () == 0) {
				char *mime = go_get_mime_type (filename);
				// ensure the found mime type is in the list
				bool found = false;
				if (mime) {
					list<string>::iterator it, itend = mime_types.end ();
					for (it = mime_types.begin (); it != itend; it++)
						if (*it == mime) {
							mime_type = *it;
							found = true;
							break;
						}
				}
				g_free (mime);
				if (!found)
					mime_type = mime_types.front ();
			}
			gtk_widget_hide (GTK_WIDGET (dialog));
			if (!App->FileProcess (filename, mime_type.c_str (), Save, GTK_WINDOW (dialog), m_pDoc)) {
				g_free (filename);
				break;
			}
			g_free (filename);
		} else {
			GSList* files = gtk_file_chooser_get_uris (chooser);
			GSList* iter = files;
			gtk_widget_hide (GTK_WIDGET (dialog));
			while (iter) {
				filename = (char*) iter->data;
				if (!mime_type.length ()) {
					char *mime = go_get_mime_type (filename);
					if (mime)
						mime_type = mime;
					g_free (mime);
				}
				App->FileProcess(filename, mime_type.c_str (), Save, GTK_WINDOW (dialog), m_pDoc);
				g_free (filename);
				iter = iter->next;
			}
			g_slist_free (files);
			break;
		}
	}
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

}	//	namespace gcu
