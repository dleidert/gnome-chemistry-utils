/* 
 * Gnome Chemistry Utils GOffice component
 * gogcuapp.h
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GO_GCU_APP_H
#define GO_GCU_APP_H

#include "gchemutils.h"
#include <string>

namespace gcu {
	class Document;
}

class GOGcuApplication
{
public:
	GOGcuApplication ();
	virtual ~GOGcuApplication ();

	virtual gcu::Document *ImportDocument (const std::string& mime_type, const char* data, int length) = 0;
	virtual GtkWindow *EditDocument (GOGChemUtilsComponent *gogcu) = 0;
};

#endif	// GO_GCU_APP_H
