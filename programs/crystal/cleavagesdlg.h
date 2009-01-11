// -*- C++ -*-

/* 
 * Gnome Crystal
 * cleavagesdlg.h 
 *
 * Copyright (C) 2002-2006 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_CLEAVAGESDLG_H
#define GCRYSTAL_CLEAVAGESDLG_H

#include <gcu/dialog.h>

class gcDocument;
class gcApplication;

struct CleavageStruct;

using namespace gcu;

class gcCleavagesDlg: public Dialog
{
public:
	gcCleavagesDlg (gcApplication *App, gcDocument* pDoc);
	virtual ~gcCleavagesDlg ();
	
	void CleavageAdd ();
	void CleavageDelete ();
	void CleavageDeleteAll ();
	void CleavageSelect (GtkTreeSelection *Selection);
	void OnEdited (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text);
	virtual bool Apply ();

private:
	char m_buf[64];
	gcDocument *m_pDoc;
	GtkListStore *CleavageList;
	GtkToggleButton *FixedBtn;
	GtkTreeSelection *Selection;
	GArray *m_Cleavages;
	GtkWidget *DeleteBtn, *DeleteAllBtn;
};

#endif //GCRYSTAL_CLEAVAGESDLG_H
