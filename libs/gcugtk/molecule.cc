// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/molecule.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include "molecule.h"

namespace gcugtk {

Molecule::Molecule (gcu::TypeId Type, gcu::ContentType ct): gcu::Molecule (Type, ct)
{
}

Molecule::Molecule (gcu::Atom* pAtom, gcu::ContentType ct): gcu::Molecule (pAtom, ct)
{
}

Molecule::~Molecule ()
{
}

}
