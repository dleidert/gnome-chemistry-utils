// -*- C++ -*-

/*
 * Gnome Crystal
 * application.h
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_APPLICATION_H
#define GCRYSTAL_APPLICATION_H

#include <list>
#include "config.h"
#include "document.h"
#include "view.h"
#include <gcr/application.h>

class gcApplication: public gcr::Application
{
public:
	gcApplication();
	~gcApplication();

//Callbacks methods
	gcr::Document *OnFileNew();
	gcr::Window *CreateNewWindow (gcr::Document *doc);

private:
};

#endif //GCRYSTAL_APPLICATION_H
