// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/chem3ddoc.h
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_CHEM3D_DOCUMENT_H
#define GCU_CHEM3D_DOCUMENT_H

#include <gcu/macros.h>
#include <gcu/gldocument.h>
#include <gcu/molecule.h>
#include <openbabel/mol.h>

/*!\file*/
namespace gcu {

/*! \enum Display3DMode
 3D display mode.
 Possible values are:
	 - BALL_AND_STICK: use ball and stick representation; atoms are represented by spheres with a radius equal to 20% of
	 their van der Waals radius and bonds are displayed as cylinders Multiple bonds are displayed as multiple cylinders.
	 - SPACEFILL: use space filling representation; atoms are represented by spheres with a radius equal
	 their van der Waals radius; bonds are not displayed.
	 - CYLINDERS: only bonds are represented as cylinders, atoms just end the cylinders.
	 - WIREFRAME: bonds are represented as narrow lines, atoms just end the lines.
*/
typedef enum
{
	BALL_AND_STICK,
	SPACEFILL,
	CYLINDERS,
	WIREFRAME
} Display3DMode;

class Application;
class Matrix;

/*!
\class Chem3dDoc gcu/chem3ddoc.h

Document class for a molecule. Embeds an OpenBabel::OBMol object.
*/
class Chem3dDoc: public GLDocument
{
public:
/*!
Default constructor
*/
	Chem3dDoc ();
/*!
@param App the application.
@param View: an optional already existing GLView instance.
*/
	Chem3dDoc (Application *App, GLView *View);
/*!
Default destructor
*/
	virtual ~Chem3dDoc ();

/*!
@param m the Matrix giving the current model orientation

Displays the molecule using OpenGL.
*/
	void Draw (Matrix const &m) const;

/*!
@return true if the molecule have no atom, false otherwise.
*/
	bool IsEmpty () {return m_Mol->GetAtomsNumber () == 0;}

/*!
@param uri the uri of the molecule file.
@param mime_type the mime type of the molecule file.

Loads a molecule from the provided uri using OpenBabel.
*/
	void Load (char const *uri, char const *mime_type);

/*!
@param data the inline data.
@param mime_type the mime type of the data.

Loads a molecule from the provided data using OpenBabel.
*/
	void LoadData (char const *data, char const *mime_type);

/*!
@param filename the name of the vrml file to which the data should be written.

Exports the embedded molecule as a vrml scene.
*/
	void OnExportVRML (std::string const &filename);

private:
	Molecule *m_Mol;

/*!\fn SetDisplay3D(Display3DMode mode)
@param mode: the new mode.

Sets the display mode to one of the available Display3DMode values.
*/
/*!\fn GetDisplay3D()
@return the current mode.
*/
/*!\fn GetRefDisplay3D()
@return the current mode as a reference.
*/
GCU_PROP (Display3DMode, Display3D);
};

}	// namespace gcu

#endif	//	GCU_CHEM3D_DOCUMENT_H
