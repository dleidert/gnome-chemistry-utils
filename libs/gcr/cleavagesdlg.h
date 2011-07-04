// -*- C++ -*-

/*
 * Gnome Crystal
 * cleavagesdlg.h
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

#ifndef GCR_CLEAVAGESDLG_H
#define GCR_CLEAVAGESDLG_H

#include <gcugtk/dialog.h>
#include "grid.h"

namespace gcu {
class Application;
}

namespace gcr {

class Document;
class Application;

struct CleavageStruct;

class CleavagesDlg: public gcugtk::Dialog
{
public:
	CleavagesDlg (gcr::Application *App, gcr::Document* pDoc);
	virtual ~CleavagesDlg ();

	void CleavageAdd ();
	void CleavageDelete ();
	void CleavageDeleteAll ();
	void CleavageSelect (GtkTreeSelection *Selection);
	void OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text);
	bool Apply ();

private:
	char m_buf[64];
	Document *m_pDoc;
	GtkListStore *CleavageList;
	GtkToggleButton *FixedBtn;
	GtkTreeSelection *Selection;
	GArray *m_Cleavages;
	GtkWidget *DeleteBtn, *DeleteAllBtn;
	GtkWidget *m_Grid;
};

}	//	namespace gcr

#endif //GCR_CLEAVAGESDLG_H
