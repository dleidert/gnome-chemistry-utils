// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/stringdlg.cc
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "stringdlg.h"
#include "application.h"
#include "message.h"
#include "window.h"
#include <gcu/document.h>
#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <sstream>
#include <cstring>

using namespace std;

namespace gcugtk {

static GtkTargetEntry const stargets[] = {
	{(char *)"STRING", 0, 0}
};
static char *copied = NULL;

static void on_copy (StringDlg *dlg)
{
	dlg->Copy ();
}

static void on_get_data(GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info, StringDlg *dlg)
{
	dlg->OnGetData (clipboard, selection_data, info);
}

static void on_string_clear_data(G_GNUC_UNUSED GtkClipboard *clipboard, G_GNUC_UNUSED StringDlg *dlg)
{
	g_free (copied);
	copied = NULL;
}

StringDlg::StringDlg (gcu::Document *pDoc, string const &data, enum data_type type):
	gcugtk::Dialog (static_cast <Application * > (pDoc->GetApplication()), UIDIR"/stringdlg.ui", "string", GETTEXT_PACKAGE)
{
	Data = data;
	Type = type;
	switch (Type) {
	case INCHI:
		gtk_window_set_title (dialog, "InChI");
		break;
	case INCHIKEY:
		gtk_window_set_title (dialog, "InChIKey");
		break;
	case SMILES:
		gtk_window_set_title (dialog, "SMILES");
		break;
	}
	View = GTK_TEXT_VIEW (GetWidget ("text"));
	Buffer = gtk_text_view_get_buffer (View);
	gtk_text_buffer_set_text (Buffer, data.c_str () , -1);
	GtkWidget *w = GetWidget ("copy");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_copy), this);
	gtk_window_set_transient_for (dialog, static_cast < Window * > (pDoc->GetWindow ())->GetWindow ());
}

StringDlg::~StringDlg ()
{
}

bool StringDlg::Apply ()
{
	GtkFileChooserDialog *dlg = (GtkFileChooserDialog*) gtk_file_chooser_dialog_new (
				_("Save as"), dialog, GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dlg), GTK_RESPONSE_OK);
	GtkFileChooser* chooser = GTK_FILE_CHOOSER (dlg);
	GtkFileFilter* filter = gtk_file_filter_new ();
	if (Type == SMILES)
		gtk_file_filter_add_pattern (filter, "*.smi");
	else
		gtk_file_filter_add_pattern (filter, "*.inchi");
	gtk_file_chooser_set_filter (chooser, filter);
	// Now add network directories
	gtk_file_chooser_set_local_only (chooser, false);
	char const* dir = reinterpret_cast<Application*> (m_App)->GetCurDir ();
	if (dir)
		gtk_file_chooser_set_current_folder_uri (chooser, dir);
	char const *filename, *ext = (Type == SMILES)? ".smi": ".inchi";
	char *filename2;
	bool err;
	while (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_uri (chooser);
		if (!filename || !strlen (filename) || filename[strlen (filename) - 1] == '/') {
			GtkWidget* message = gtk_message_dialog_new (dialog, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
																_("Please enter a file name,\nnot a directory"));
			gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
			gtk_dialog_run (GTK_DIALOG (message));
			gtk_widget_destroy (message);
			continue;
		}
		if (strlen (ext) > strlen (filename) ||
			strcmp (filename + strlen (filename) - strlen (ext), ext))
			filename2 = g_strconcat (filename, ext, NULL);
		else
			filename2 = g_strdup (filename);
		GFile *file = g_file_new_for_uri (filename2);
		err = g_file_query_exists (file, NULL);
		int result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename2, NULL);
			char *message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			Message *box = new Message (static_cast < gcugtk::Application * > (GetApp ()),
			                            message, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, GetWindow ());
			result = box->Run ();
			g_free (message);
		}
		if (result == GTK_RESPONSE_YES)
		{
			// destroy the old file
			GError *error = NULL;
			if (err) {
				g_file_delete (file, NULL, &error);
				if (error) {
					char *unescaped = g_uri_unescape_string (filename2, NULL);
					char *message = g_strdup_printf (_("Error while processing %s:\n%s"), unescaped, error->message);
					g_free (unescaped);
					g_error_free (error);
					Message *box = new Message (static_cast < gcugtk::Application * > (GetApp ()),
					                            message, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, GetWindow ());
					result = box->Run ();
					g_free (message);
					g_object_unref (file);
					continue;
				}
			}
			ostringstream ofs;
			GOutputStream *output = G_OUTPUT_STREAM (g_file_create (file, G_FILE_CREATE_NONE, NULL, &error));
			if (error) {
				char *unescaped = g_uri_unescape_string (filename2, NULL);
				char *message = g_strdup_printf (_("Could not open file %s, error was:\n%s"), unescaped, error->message);
				g_free (unescaped);
				Message *box = new Message (static_cast < gcugtk::Application * > (GetApp ()),
				                            message, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, GetWindow ());
				box->Run ();
				g_free (message);
				g_error_free (error);
				g_object_unref (file);
				continue;
			}
			ofs << Data;
			ofs << endl;
			gsize nb = ofs.str ().size (), n = 0;
			while (n < nb) {
				n += g_output_stream_write (output, ofs.str ().c_str () + n, nb - n, NULL, &error);
				if (error) {
					char *unescaped = g_uri_unescape_string (filename2, NULL);
					char *message = g_strdup_printf (_("Could not write to file %s, error was:\n%s."), unescaped, error->message);
					g_free (unescaped);
					Message *box = new Message (static_cast < gcugtk::Application * > (GetApp ()),
						                        message, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, GetWindow ());
					box->Run ();
					g_free (message);
					g_error_free (error);
					g_object_unref (file);
					continue;
				}
			}
			g_output_stream_close (output, NULL, &error);
			g_object_unref (file);
			if (error) {
				char *unescaped = g_uri_unescape_string (filename2, NULL);
				char *message = g_strdup_printf (_("Could not close file %s, error was:\n%s"), unescaped, error->message);
				g_free (unescaped);
				Message *box = new Message (static_cast < gcugtk::Application * > (GetApp ()),
				                            message, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, GetWindow ());
				box->Run ();
				g_free (message);
				g_error_free (error);
				continue;
			}
			break;
		}
		g_free (filename2);
	}
	gtk_widget_destroy (GTK_WIDGET (dlg));
	return true;
}

void StringDlg::Copy ()
{
	GtkClipboard* clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	gtk_clipboard_set_with_data (clipboard, stargets, 1,
		(GtkClipboardGetFunc) on_get_data,
		(GtkClipboardClearFunc) on_string_clear_data, this);
	gtk_clipboard_request_contents (clipboard,
			gdk_atom_intern ("TARGETS", FALSE),
			(GtkClipboardReceivedFunc) Application::OnReceiveTargets,
			m_App);
	g_free (copied); // copied should already be NULL,btw
	copied = g_strdup (Data.c_str ());
}

void StringDlg::OnGetData (G_GNUC_UNUSED GtkClipboard *clipboard, GtkSelectionData *selection_data,  G_GNUC_UNUSED guint info)
{
	gtk_selection_data_set_text (selection_data, copied, strlen (copied));
}

}	//	namespace gcugtk
