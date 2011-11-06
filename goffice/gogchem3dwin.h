/*
 * GChemPaint GOffice component
 * gogchem3dwin.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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


#ifndef GCP_GCHEM3D_WIN_H
#define GCP_GCHEM3D_WIN_H

#include <gcugtk/chem3dwindow.h>
#include "gchemutils.h"

class GOGChem3dApplication;

class GOGChem3dWindow: public gcugtk::Chem3dWindow
{
public:
	GOGChem3dWindow (GOGChem3dApplication *App, GOGChemUtilsComponent *gogcu);
	virtual ~GOGChem3dWindow ();

	void OnSave ();
	char const *GetDefaultTitle ();

private:
	GOGChemUtilsComponent *m_gogcu;
};

#endif	//	GCP_GCHEM3D_WIN_H
