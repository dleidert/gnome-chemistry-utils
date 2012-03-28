// -*- C++ -*-

/*
 * Gnome Crystal
 * view.h
 *
 * Copyright (C) 2000-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_VIEW_H
#define GCRYSTAL_VIEW_H

#include <libxml/parser.h>
#include <list>
#include <gcr/view.h>
#include <gcu/matrix.h>
#include <gcu/dialog.h>
#include <gcu/dialog-owner.h>

class gcDocument;
class gcApplication;
class gcWindow;

class gcView: public gcr::View
{
public:
	gcView (gcDocument *pDoc);
	gcView (gcView *pView);
	~gcView ();

	bool LoadOld (xmlNodePtr node);

private:

	GtkMenuItem* m_pMenu;
};

#endif //GCRYSTAL_VIEW_H
