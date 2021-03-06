// -*- C++ -*-

/*
 * Gnome Crystal
 * atom.cc
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

#include "config.h"
#include "gcrystal.h"
#include "atom.h"
#include <gcu/element.h>
#include <glib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <cstring>

gcAtom::gcAtom (): gcr::Atom ()
{
}

gcAtom::~gcAtom ()
{
}

gcAtom::gcAtom (int Z, double x, double y, double z): gcr::Atom (Z, x, y, z)
{
}

gcAtom::gcAtom (gcAtom& caAtom): gcr::Atom ((gcr::Atom&) caAtom)
{
}

gcAtom& gcAtom::operator= (gcAtom& caAtom)
{
	(gcr::Atom&) *this = (gcr::Atom&) caAtom;
	return *this ;
}

bool gcAtom::LoadOld (xmlNodePtr node, unsigned version)
{
	char *txt;
	xmlNodePtr child = node->children;
	while (child) {
		if (!strcmp ((char *) child->name, "element")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				m_Z = gcu::Element::Z (txt);
				xmlFree (txt);
			}
		} else if (!strcmp ((char *) child->name, "position")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				sscanf (txt, "%lg %lg %lg", &m_x, &m_y, &m_z);
				xmlFree (txt);
			}
		} else if (!strcmp ((char *) child->name, "color")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				if (version < 0x200)
					sscanf (txt, "%g %g %g %g", &m_fBlue, &m_fRed, &m_fGreen, &m_fAlpha);
				else
					sscanf (txt, "%g %g %g %g", &m_fRed, &m_fGreen, &m_fBlue, &m_fAlpha);
				xmlFree(txt);
			}
		} else if (!strcmp((char *)child->name, "radius")) {
			txt = (char*) xmlNodeGetContent (child);
			if (txt) {
				sscanf (txt, "%lg", &m_Radius.value.value);
				xmlFree (txt);
			}
		}
		child = child->next;
	}
	if (m_Radius.value.value <= 0)
		return false;
	return true;
}
