// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystaldoc.h 
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef CRYSTAL_DOC_H
#define CRYSTAL_DOC_H

#include <libxml/tree.h>
#include <glib.h>
#include "chemistry.h"
#include "crystalatom.h"
#include "crystalbond.h"
#include "crystalline.h"
#include "crystalcleavage.h"
#include "document.h"
#include "macros.h"
#include <gcu/gldocument.h>

/*!\file*/
namespace gcu
{

class CrystalView;
class Matrix;
class SpaceGroup;

/*!\enum gcLattices crystalviewer/crystaldoc.h
This enumeration gives sympolic names to the fourteen Bravais lattices.

Possible values are:
- cubic
- body_centered_cubic
- face_centered_cubic
- hexagonal
- tetragonal
- body_centered_tetragonal
- orthorhombic
- base_centered_orthorhombic
- body_centered_orthorhombic
- face_centered_orthorhombic
- rhombohedral
- monoclinic
- base_centered_monoclinic
- triclinic
*/
enum gcLattices {cubic=0,
				 body_centered_cubic,
				 face_centered_cubic,
				 hexagonal,
				 tetragonal,
				 body_centered_tetragonal,
				 orthorhombic,
				 base_centered_orthorhombic,
				 body_centered_orthorhombic,
				 face_centered_orthorhombic,
				 rhombohedral,
				 monoclinic,
				 base_centered_monoclinic,
				 triclinic
};

/*!\class CrystalDoc gcu/crystaldoc.h
The document containing the crystal structure.
*/				 
class CrystalDoc: public GLDocument
{
public:
/*!
The constructor of CrystalDoc
*/
	CrystalDoc (Application *App);
/*!
The destructor of CrystalDoc
*/
	virtual ~CrystalDoc ();

/*!
@param xml: a pointer to the root xmlNode of the xmlDoc containing the definition of the crystal.

Analyses the contents of the XML document and builds the cryatl structure from the data. Typical usage is:
\code
CrystalDoc* crystal = new CrystalDoc();
xmlDocPtr doc = xmlParseFile(filename);
crystal->ParseXMLTree(doc->children);
\endcode
*/
	void ParseXMLTree (xmlNode* xml);
/*!
This method must be called when a new document is loaded or when the definition of the crystal is changed. It recalculates
everything and updates all the views.
*/
	void Update ();
/*!
@return a pointer to the first CrystalView of the document. The view will be created if it does not already exist.
*/
	CrystalView* GetView ();

/*!
@param m the Matrix giving the current model orientation

Displays the molecule using OpenGL.
*/
	void Draw (Matrix const &m) const;

/*!
Creates a view of the document. This method should be overrided by programs deriving a new view class from
CrystalView.

@return a pointer to the new CrystalView instance.
*/
	virtual CrystalView* CreateNewView ();
/*!
Creates a new atom. This method should be overrided by programs deriving a new atom class from
CrystalAtom.

@return a pointer to the new CrystalAtom instance.
*/
	virtual CrystalAtom* CreateNewAtom ();
/*!
Creates a new line. This method should be overrided by programs deriving a new view class from
CrystalLine.

@return a pointer to the new CrystalLine instance.
*/
	virtual CrystalLine* CreateNewLine ();
/*!
Creates a new cleavage. This method should be overrided by programs deriving a new line class from
CrystalCleavage
@return a pointer to the new CrystalCleavage instance.
*/
	virtual CrystalCleavage* CreateNewCleavage ();
/*!
Builds the xmlDoc corresponding to the crystal structure.
@return a pointer to the XML document.
*/
	xmlDocPtr BuildXMLTree () const;
/*!
@return the identity of the program as saved in files in the generator tag. This method should be overrided
by programs able to save crystal structures in XML files conforming to gcrystal.dtd. It is used mainly to ensure
compatiblity with files created by older versions of the program.
*/
	virtual const char* GetProgramId () const;

/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties to the document
@return true if the property could be set, or if the property is not relevant, false otherwise.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
@param property the property id as defined in objprops.h

Used when saving to get properties from the document.
@return the property as a string. The returned string might be empty.
*/
	std::string GetProperty (unsigned property) const;

/*!
Called by the application whe the document has been loaded to update the title
and add some lines.
*/
	bool Loaded () throw (LoaderError);
/*!
	@param object the Object instance to add as a child.
*/
	void AddChild (Object* object);

	SpaceGroup const *FindSpaceGroup ();
/*!
Reinitialize a CrystalDoc instance. Used when loading a file in an already existing document.
*/
	void Reinit ();

protected:
/*!
Initialize a new CrystalDoc instance.
*/
	void Init ();
/*!
@param node: the xmlNode containing the serialized view.

Loads a view from a XML document. This methd must be overrided by applications supporting multiple views.
*/
	virtual bool LoadNewView (xmlNodePtr node);

private:
	void Duplicate (CrystalAtom& Atom);
	void Duplicate (CrystalLine& Line);

protected:
/*!
The Bravais lattice of the crystal.
*/
	gcLattices m_lattice;
/*!
The a parameter of the unit cell.
*/
	gdouble m_a;
/*!
The b parameter of the unit cell.
*/
	gdouble m_b;
/*!
The c parameter of the unit cell.
*/
	gdouble m_c;
/*!
The alpha angle of the unit cell.
*/
	gdouble m_alpha;
/*!
The beta angle of the unit cell.
*/
	gdouble m_beta;
/*!
The gamma angle of the unit cell.
*/
	gdouble m_gamma;
/*!
The minimum x coordinate in the representation of the crystal structure.
*/
	gdouble m_xmin;
/*!
The minimum y coordinate in the representation of the crystal structure.
*/
	gdouble m_ymin;
/*!
The minimum z coordinate in the representation of the crystal structure.
*/
	gdouble m_zmin;
/*!
The maximum x coordinate in the representation of the crystal structure.
*/
	gdouble m_xmax;
/*!
The maximum y coordinate in the representation of the crystal structure.
*/
	gdouble m_ymax;
/*!
The maximum z coordinate in the representation of the crystal structure.
*/
	gdouble m_zmax;
/*!
true if cleavages must not change positions in the view.
*/
	gboolean m_bFixedSize;  //true if cleavages must not change positions in the view
/*!
List of the atoms in the definition of the crystal
*/
	CrystalAtomList AtomDef;
/*!
List of the atoms displayed.
*/
	CrystalAtomList Atoms;
/*!
List of the lines in the definition of the crystal
*/
	CrystalLineList LineDef;
/*!
List of the lines displayed.
*/
	CrystalLineList Lines;
/*!
List of the cleavages defined.
*/
	CrystalCleavageList Cleavages;
/*!
List of the views of the document.
*/
	std::list <CrystalView *> m_Views;

/*!GetSpaceGroup()
@return the space group associated with the lattice.
*/

GCU_RO_PROP (std::string, NameCommon);
GCU_RO_PROP (std::string, NameSystematic);
GCU_RO_PROP (std::string, NameMineral);
GCU_RO_PROP (std::string, NameStructure);
GCU_RO_PROP (SpaceGroup const *, SpaceGroup);
};

extern gchar const *LatticeName[];

} //namespace gcu

#endif //CRYSTAL_DOC_H
