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
#include <goffice/utils/go-file.h>

using namespace gcu;

FileChooser::FileChooser (Application *App, bool Save,list<char const*> mime_types, Document *pDoc, char const *title)
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
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER (dialog);
	GtkFileFilter* filter = gtk_file_filter_new ();
	list<char const*>::iterator i, iend = mime_types.end ();
	for (i = mime_types.begin (); i != iend; i++)
		gtk_file_filter_add_mime_type (filter, *i);
	GtkComboBox *format_combo = NULL;
	if (!Save)
		gtk_file_chooser_set_select_multiple (chooser, true);
	if (mime_types.size () > 1) {
		GtkWidget *box = gtk_hbox_new (FALSE, 2);
		GtkWidget *label = gtk_label_new_with_mnemonic (_("File _type:"));
		format_combo = GTK_COMBO_BOX (gtk_combo_box_new_text ());
		gtk_combo_box_append_text (format_combo, _("Automatic"));
		for (i = mime_types.begin (); i != iend; i++)
			gtk_combo_box_append_text (format_combo, go_mime_type_get_description (*i));
		gtk_combo_box_set_active (format_combo, 0);

		gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 6);
		gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (format_combo), FALSE, TRUE, 6);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (format_combo));

		gtk_file_chooser_set_extra_widget (dialog, box);
		gtk_widget_show_all (box);
	}
	// Now add network directories
	gtk_file_chooser_set_local_only (chooser, false);
	char const* dir = App->GetCurDir ();
	if (dir)
		gtk_file_chooser_set_current_folder_uri (chooser, dir);
	while (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		// find the mime_type
		char const *mime_type = NULL;
		if (mime_types.size () == 1)
			mime_type = mime_types.front ();
		else if (mime_types.size () > 0) {
			int j = gtk_combo_box_get_active (format_combo);
			if (j > 0) {
				i = mime_types.begin ();
				while (--j > 0)
					i++;
				mime_type = *i;
			}
		}
		if (!mime_type)
			mime_type = go_get_mime_type (filename);
		if (Save) {
			filename = gtk_file_chooser_get_uri (chooser);
			if (!App->FileProcess (filename, mime_type, Save, GTK_WINDOW (dialog), m_pDoc)) {
				g_free (filename);
				break;
			}
			g_free (filename);
		} else {
			GSList* files = gtk_file_chooser_get_uris (chooser);
			GSList* iter = files;
			while (iter) {
				filename = (char*) iter->data;
				if (!App->FileProcess(filename, mime_type, Save, GTK_WINDOW (dialog), m_pDoc)) {
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
