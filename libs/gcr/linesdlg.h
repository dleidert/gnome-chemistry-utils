// -*- C++ -*-

/*
 * Gnome Crystal
 * linesdlg.h
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCR_LINESDLG_H
#define GCR_LINESDLG_H

#include <gcugtk/dialog.h>

namespace gcr {

class Document;
class Application;

class LinesDlg: public gcugtk::Dialog
{
public:
	LinesDlg (Application *App, Document* pDoc);
	virtual ~LinesDlg ();

	virtual bool Apply ();
	void LineAdd ();
	void LineDelete ();
	void LineDeleteAll ();
	void LineSelect (GtkTreeSelection *Selection);
	void OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text);
	void OnToggled (GtkCellRendererToggle *cell, const gchar *path_string);
	void OnToggledSpecial (int Type);

private:
	char m_buf[64];
	Document *m_pDoc;
	GtkListStore *LineList;
	GtkTreeSelection *Selection;
	GtkColorButton *LineColor, *EdgesColor, *MediansColor, *DiagsColor;
	GtkEntry *LineR, *EdgesR, *MediansR, *DiagsR;
	GtkCheckButton *Edges, *Medians, *Diags;
	GtkWidget *DeleteBtn, *DeleteAllBtn;
	GArray *m_Lines;
	gint m_LineSelected;
	GtkTreeIter m_Iter;
};

}	//	namespace gcr

#endif //GCR_LINESDLG_H
