// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystaldoc.h 
 *
 * Copyright (C) 2002-2004
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
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

namespace gcu
{

class CrystalView;

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
				 triclinic};

/*!\class CrystalDoc gcu/crystaldoc.h
The document containing the crystal structure.
*/				 
class CrystalDoc
{
public:
/*!
The constructor of CrystalDoc
*/
	CrystalDoc();
/*!
The destructor of CrystalDoc
*/
	virtual ~CrystalDoc();

/*!
@param xml: a pointer to the root xmlNode of the xmlDoc containing the definition of the crystal.

Analyses the contents of the XML document and builds the cryatl structure from the data. Typical usage is:
\code
CrystalDoc* crystal = new CrystalDoc();
xmlDocPtr doc = xmlParseFile(filename);
crystal->ParseXMLTree(doc->children);
\endcode
*/
	void ParseXMLTree(xmlNode* xml);
/*!
This method must be called when a new document is loaded or when the definition of the crystal is changed. It recalculates
everything and updates all the views.
*/
	void Update();
/*!
@return a pointer to the first CrystalView of the document. The view will be created if it does not already exist.
*/
	CrystalView* GetView();
/*!
@return true if the document has been modified, false if not.
*/
	bool IsDirty() {return m_bDirty;}
/*!
Signals the document as modified since the last saving operation.
*/
	virtual void SetDirty();
/*!
Draws the document using OpenGL primitives.
*/
	void Draw();
/*!
@return the largest distance from an object displayed in the document to the center of the model.
*/
	gdouble GetMaxDist() {return m_dDist;}
/*!
Creates a view of the document. This method should be overrided by programs deriving a new view class from
CrystalView.

@return a pointer to the new CrystalView instance.
*/
	virtual CrystalView* CreateNewView();
/*!
Creates a new atom. This method should be overrided by programs deriving a new view class from
CrystalAtom.

@return a pointer to the new CrystalAtom instance.
*/
	virtual CrystalAtom* CreateNewAtom();
/*!
Creates a new line. This method should be overrided by programs deriving a new view class from
CrystalLine.

@return a pointer to the new CrystalLine instance.
*/
	virtual CrystalLine* CreateNewLine();
/*!
Creates a new cleavage. This method should be overrided by programs deriving a new line class from
CrystalCleavage
@return a pointer to the new CrystalCleavage instance.
*/
	virtual CrystalCleavage* CreateNewCleavage();
/*!
Builds the xmlDoc corresponding to the crystal structure.
@return a pointer to the XML document.
*/
	xmlDocPtr BuildXMLTree();
/*!
@return the identity of the program as saved in files in the generator tag. This method should be overrided
by programs able to save crystal structures in XML files conforming to gcrystal.dtd. It is used mainly to ensure
compatiblity with files created by older versions of the program.
*/
	virtual const char* GetProgramId();
	
protected:
/*!
Initialize a new CrystalDoc instance.
*/
	void Init();
/*!
Reinitialize a CrystalDoc instance. Used when loading a file in an already existing document.
*/
	void Reinit();
/*!
@param node: the xmlNode containing the serialized view.

Loads a view from a XML document. This methd must be overrided by applications supporting multiple views.
*/
	virtual bool LoadNewView(xmlNodePtr node);

private:
	void Duplicate(CrystalAtom& Atom);
	void Duplicate(CrystalLine& Line);

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
The maximum distance between an object and the center.
*/
	gdouble m_dDist; //maximum distance between an object and the center
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
	list <CrystalView *> m_Views;
/*!
true if the document has changed since the last saving. Changing the orientation of the model
in one of the views is considered as a change.
*/
	bool m_bDirty;
/*!
true if the document does not contain anything displayable.
*/
	bool m_bEmpty;
};
	
} //namespace gcu

#endif //CRYSTAL_DOC_H
