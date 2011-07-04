// -*- C++ -*-

/*
 * Gnome Crystal
 * atom.h
 *
 * Copyright (C) 2000-2010 Jean Br�fort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_ATOM_H
#define GCRYSTAL_ATOM_H

#include <list>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>
#include <gcr/atom.h>

class gcAtom: public gcr::Atom
{
public:
	gcAtom ();
	virtual ~gcAtom ();

public :
	gcAtom (int Z, double x, double y, double z);
	gcAtom (gcAtom& caAtom);
	gcAtom& operator= (gcAtom&);

	bool LoadOld (xmlNodePtr node, unsigned version);
};

#endif // GCRYSTAL_ATOM_H
