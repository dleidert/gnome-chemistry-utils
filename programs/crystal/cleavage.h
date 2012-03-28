// -*- C++ -*-

/*
 * Gnome Crystal
 * cleavage.h
 *
 * Copyright (C) 2001-2010 Jean Br�fort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_CLEAVAGE_H
#define GCRYSTAL_CLEAVAGE_H

#include <gcr/cleavage.h>
#include <libxml/parser.h>
#include <list>

class gcCleavage: public gcr::Cleavage
{
public:
	gcCleavage ();
	virtual ~gcCleavage ();

	bool LoadOld (xmlNodePtr node);
};

#endif //GCRYSTAL_CLEAVAGE_H
