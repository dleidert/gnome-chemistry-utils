// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/molecule.h
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


#ifndef GCU_GTK_MOLECULE_H
#define GCU_GTK_MOLECULE_H

#include <gcu/molecule.h>

/*!\file*/
namespace gcugtk {

/*!\class Molecule gcugtk/molecule.h
Provide some uiser interface for molecules.
*/
class Molecule: public gcu::Molecule
{
friend class MoleculePrivate;
public:
/*!
@param Type the type id of a derived class
@param ct the content type for the molecule (2d or 3d), defaults to
ContentType3D.

The constructor. The type id may be changed in derived classes, otherwise the
argument is not needed, since the default value is enough.
*/
	Molecule (gcu::TypeId Type = gcu::MoleculeType, gcu::ContentType ct = gcu::ContentType3D);
/*!
@param pAtom
@param ct the content type for the molecule (2d or 3d), defaults to
ContentType3D.

Builds a molecule from an atom. If the atom has bonds, the connectivity will
be explored and all atoms and bonds found will be added to the molecule.
*/
	Molecule (gcu::Atom* pAtom, gcu::ContentType ct = gcu::ContentType3D);
/*!
The destructor.
*/
	virtual ~Molecule ();

/*!
@param UIManager a GtkUIManager.
@param path the path to insert before each new action entry.
@param path_end the path to insert after each new action entry.

Adds menu entry for databases access.
*/
	void BuildDatabasesMenu (GtkUIManager *UIManager, char const *path, char const *path_end);
};

}	//	namespace gcu

#endif	//	GCU_GTK_MOLECULE_H
