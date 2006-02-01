// -*- C++ -*-

/* 
 * GChemPaint
 * filesel.cc 
 *
 * Copyright (C) 2001-2003
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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
#include "globals.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "filesel.h"
#include "view.h"

extern "C"
{
	static void on_ok(GtkWidget *widget, gcFileSel* pBox)
	{
		pBox->Apply();
	}

	static void on_cancel(GtkWidget *widget, gcFileSel* pBox)
	{
		pBox->Destroy();
	}
	
	static bool on_destroy(GtkWidget *widget, gcFileSel* pBox)
	{
		delete pBox;
		return true;
	}
}

gcFileSel::gcFileSel(const gchar* title, void (*cb)(const gchar*, gcView*), bool Save, const gchar* ext, gcView* pView, bool bModal)
{
	fsel = GTK_FILE_SELECTION(gtk_file_selection_new(title));
	m_cb = cb;
	m_pView = pView;
	m_pView->Lock();
	m_bSave = Save;
	if (ext) m_ext = g_strdup(ext);
	else m_ext = NULL;
	if (bModal || Save) gtk_window_set_modal(GTK_WINDOW(fsel), true);
	g_signal_connect(G_OBJECT(fsel->ok_button), "clicked", GTK_SIGNAL_FUNC(on_ok), this);
	g_signal_connect(G_OBJECT(fsel->cancel_button), "clicked", GTK_SIGNAL_FUNC(on_cancel), this);
	g_signal_connect(G_OBJECT(fsel), "destroy", GTK_SIGNAL_FUNC(on_destroy), this);
	gtk_widget_show(GTK_WIDGET(fsel));
}

gcFileSel::~gcFileSel()
{
	m_pView->Unlock();
	if (m_ext) g_free(m_ext);
}

void gcFileSel::Destroy()
{
	gtk_widget_destroy(GTK_WIDGET(fsel));
}

bool gcFileSel::Apply()
{
	const gchar* filename = gtk_file_selection_get_filename(fsel);
	if (filename[strlen(filename) - 1] == '/')
	{
		GtkWidget* message = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
																	_("Please enter a file name,\nnot a directory"));
		g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
		gtk_widget_show(message);
		return true;
	}
	gchar* filename2;
	struct stat buf;
	gint err;
	if (m_bSave)
	{
		if (m_ext)
		{
			int i = strlen(filename) - strlen(m_ext);
			if ((i > 0) && (!strcmp(filename +i, m_ext)))
				filename2 = g_strdup(filename);
			else
				filename2 = g_strdup_printf("%s%s", filename, m_ext);
		}
		else filename2 = g_strdup(filename);
		err = stat(filename2, &buf);
		gint result = GTK_RESPONSE_YES;
		if (!err)
		{
			gchar * message = g_strdup_printf(_("File %s\nexists, overwrite?"), filename2);
			GtkDialog* Box = GTK_DIALOG(gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			result = gtk_dialog_run(Box);
			gtk_widget_destroy(GTK_WIDGET(Box));
			g_free(message);
		}
		if (result == GTK_RESPONSE_YES)
		{
			gtk_widget_hide(GTK_WIDGET(fsel));
			m_cb(filename2, m_pView);
		}
		gtk_widget_destroy(GTK_WIDGET(fsel));
	}
	else //loading
	{
		err = stat(filename, &buf);
		if (err) filename2 = g_strdup_printf("%s%s",filename, m_ext);
		else filename2 = g_strdup(filename);
		gtk_widget_hide(GTK_WIDGET(fsel));
		m_cb(filename2, m_pView);
		gtk_widget_destroy(GTK_WIDGET(fsel));
	}
	g_free(filename2);
}
