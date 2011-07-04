// -*- C++ -*-

/*
 * Gnome Chemisty Utils
 * gcr/document.h
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_DOCUMENT_H
#define GCR_DOCUMENT_H

#include <libxml/tree.h>
#include <glib.h>
#include "atom.h"
#include "bond.h"
#include "line.h"
#include "cleavage.h"
#include <gcu/chemistry.h>
#include <gcu/macros.h>
#include <gcu/gldocument.h>

namespace gcu {
class Application;
class Matrix;
class SpaceGroup;
}

/*!\file*/
namespace gcr {

class View;

/*!\enum Lattice gcr/document.h
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
enum Lattice {
	cubic=0,
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

/*!\class Document gcr/document.h
The document containing the crystal structure.
*/
class Document: public gcu::GLDocument
{
public:
/*!
The constructor of Document
*/
	Document (gcu::Application *App);
/*!
The destructor of Document
*/
	virtual ~Document ();

/*!
@param xml: a pointer to the root xmlNode of the xmlDoc containing the definition of the crystal.

Analyses the contents of the XML document and builds the cryatl structure from the data. Typical usage is:
\code
Document *crystal = new Document();
xmlDocPtr doc = xmlParseFile (filename);
crystal->ParseXMLTree (doc->children);
\endcode
*/
	void ParseXMLTree (xmlNode* xml);
/*!
This method must be called when a new document is loaded or when the definition of the crystal is changed. It recalculates
everything and updates all the views.
*/
	void Update ();
/*!
@return a pointer to the first View of the document. The view will be created if it does not already exist.
*/
	View* GetView ();

/*!
@param m the Matrix giving the current model orientation

Displays the molecule using OpenGL.
*/
	void Draw (gcu::Matrix const &m) const;

/*!
Creates a view of the document. This method should be overrided by programs deriving a new view class from
View.

@return a pointer to the new View instance.
*/
	virtual View* CreateNewView ();
/*!
Creates a new atom. This method should be overrided by programs deriving a new atom class from
Atom.

@return a pointer to the new Atom instance.
*/
	virtual Atom* CreateNewAtom ();
/*!
Creates a new line. This method should be overrided by programs deriving a new view class from
Line.

@return a pointer to the new Line instance.
*/
	virtual Line* CreateNewLine ();
/*!
Creates a new cleavage. This method should be overrided by programs deriving a new line class from
Cleavage
@return a pointer to the new Cleavage instance.
*/
	virtual Cleavage* CreateNewCleavage ();
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
Called by the application when the document has been loaded to update the title
and add some lines.
*/
	bool Loaded () throw (gcu::LoaderError);
/*!
	@param object the Object instance to add as a child.
*/
	void AddChild (Object* object);
/*!
Attempts to infer the symmetry space group for the crystal.
@return the SpaceGroup found.
*/
	gcu::SpaceGroup const *FindSpaceGroup ();
/*!
Reinitialize a Document instance. Used when loading a file in an already existing document.
*/
	void Reinit ();
	AtomList* GetAtomList () {return &AtomDef;}
	CleavageList *GetCleavageList () {return &Cleavages;}
	LineList* GetLineList () {return &LineDef;}
	void GetCell (Lattice *lattice, double *a, double *b, double *c, double *alpha, double *beta, double *gamma);
	void SetCell (Lattice lattice, double a, double b, double c, double alpha, double beta, double gamma);
	void GetSize (double* xmin, double* xmax, double* ymin, double* ymax, double* zmin, double* zmax);
	void SetSize (double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

protected:
/*!
Initialize a new Document instance.
*/
	void Init ();
/*!
@param node: the xmlNode containing the serialized view.

Loads a view from a XML document. This methd must be overrided by applications supporting multiple views.
*/
	virtual bool LoadNewView (xmlNodePtr node);

private:
	void Duplicate (Atom& Atom);
	void Duplicate (Line& Line);

protected:
/*!
The Bravais lattice of the crystal.
*/
	Lattice m_lattice;
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
List of the atoms in the definition of the crystal
*/
	AtomList AtomDef;
/*!
List of the atoms displayed.
*/
	AtomList Atoms;
/*!
List of the lines in the definition of the crystal
*/
	LineList LineDef;
/*!
List of the lines displayed.
*/
	LineList Lines;
/*!
List of the cleavages defined.
*/
	CleavageList Cleavages;
/*!
List of the views of the document.
*/
	std::list <View *> m_Views;

/*!\fn GetNameCommon()
@return the common name or the chemical entity.
*/
GCU_RO_PROP (std::string, NameCommon)
/*!\fn GetNameSystematic()
@return the systematic name or the chemical entity.
*/
GCU_RO_PROP (std::string, NameSystematic)
/*!\fn GetNameMineral()
@return the mineral name or the chemical entity.
*/
GCU_RO_PROP (std::string, NameMineral)
/*!\fn GetNameStructure()
@return the structure name or the chemical entity.
*/
GCU_RO_PROP (std::string, NameStructure)
/*!\fn SetSpaceGroup(SpaceGroup const *group)
@param group a SpaceGroup.
Associates a the space group with the lattice.
*/
/*!\fn GetSpaceGroup()
@return the space group associated with the lattice.
*/
/*!\fn GetRefSpaceGroup()
@return the space group associated with the lattice as a reference.
*/
GCU_PROP (gcu::SpaceGroup const *, SpaceGroup)
/*!\fn SetAutoSpaceGroup(bool auto)
@param auto wheteher the lattice SpaceGroup should be automatically searched for.

If true, after each change, the framework will reevaluate the space group according
to the Bravais lattice and the defines atoms.
*/
/*!\fn GetAutoSpaceGroup()
@return whether the space group is automatically evaluated.
*/
/*!\fn GetRefAutoSpaceGroup()
@return whether the space group is automatically evaluated as a reference.
*/
GCU_PROP (bool, AutoSpaceGroup)

/*!\var m_FixedSize
true if cleavages must not change positions in the view.
*/
GCU_PROP (bool, FixedSize);
};

/*!
A table of the Bravais lattices names.
*/
extern gchar const *LatticeName[];

} // namespace gcr

#endif // GCR_DOCUMENT_H
