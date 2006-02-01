// -*- C++ -*-

/* 
 * Gnome Crystal
 * filesel.h 
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

#ifndef GCRYSTAL_FILESEL_H
#define GCRYSTAL_FILESEL_H

#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include "document.h"

class gcFileSel
{
public:
	gcFileSel(const gchar* title, void (*cb)(const gchar*, gcView*), bool Save, const gchar* ext = NULL, gcView* pView = NULL, bool bModal = false);
	virtual ~gcFileSel();

	virtual void Destroy();
	virtual bool Apply();
	
private:
	GtkFileSelection* fsel;
	void (*m_cb)(const gchar*, gcView*);
	gchar* m_ext;
	bool m_bSave;
	gcView* m_pView;
};

#endif // GCRYSTAL_FILESEL_H
