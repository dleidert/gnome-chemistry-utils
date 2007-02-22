// -*- C++ -*-

/* 
 * Gnome Crystal
 * sizedlg.h 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GCRYSTAL_SIZEDLG_H
#define GCRYSTAL_SIZEDLG_H

#include <gcu/dialog.h>

class gcDocument;
class gcApplication;

using namespace gcu;

class gcSizeDlg: public Dialog
{
public:
	gcSizeDlg (gcApplication *App, gcDocument* pDoc);
	virtual ~gcSizeDlg ();
	
	virtual bool Apply ();

private:
	char m_buf[64];
	gcDocument *m_pDoc;
	GtkEntry *MaxX, *MinX, *MaxY, *MinY, *MaxZ, *MinZ;
};

#endif //GCRYSTAL_SIZEDLG_H