/*
 * Gnome Chemistry Utils GOffice component
 * gogchem3dapp.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GO_GCHEM3D_APP_H
#define GO_GCHEM3D_APP_H

#include "gchemutils.h"
#include "gogcuapp.h"
#include <gcugtk/application.h>

namespace gcu {
	class Document;
}

class GOGChem3dApplication: public gcugtk::Application, public GOGcuApplication
{
public:
	GOGChem3dApplication ();
	virtual ~GOGChem3dApplication ();

	void ImportDocument (GOGChemUtilsComponent *gogcu);
	GtkWindow *EditDocument (GOGChemUtilsComponent *gogcu);
	bool GetData (GOGChemUtilsComponent *gogcu, gpointer *data, int *length, void (**clearfunc) (gpointer), gpointer *user_data);
	void Render (GOGChemUtilsComponent *gogcu, cairo_t *cr, double width, double height);
	void UpdateBounds (GOGChemUtilsComponent *gogcu);
	gcu::ContentType GetContentType () {return gcu::ContentType3D;}
};

#endif	// GO_GCHEM3D_APP_H
