// -*- C++ -*-

/*
 * Gnome Chemisty Utils
 * gcr/document.h
 *
 * Copyright (C) 2002-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

	bool Load (const std::string &filename);

/*!
@param xml: a pointer to the root xmlNode of the xmlDoc containing the definition of the crystal.

Analyses the contents of the XML document and builds the crystal structure from the data. Typical usage is:
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

	void UpdateAllViews ();

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

/*!
@return the list of atoms defining the crystal.
*/
	AtomList* GetAtomList () {return &AtomDef;}

/*!
@return the list of defined cleavages.
*/
	CleavageList *GetCleavageList () {return &Cleavages;}

/*!
@return the list of defined lines.
*/
	LineList* GetLineList () {return &LineDef;}

/*!
@param lattice where to store the lattice.
@param a where to store the a cell parameter.
@param b where to store the b cell parameter.
@param c where to store the c cell parameter.
@param alpha where to store the alpha cell parameter.
@param beta where to store the beta cell parameter.
@param gamma where to store the gamma cell parameter.

Retrieves the cell parameters.
*/
	void GetCell (Lattice *lattice, double *a, double *b, double *c, double *alpha, double *beta, double *gamma);

/*!
@param lattice the new lattice.
@param a the new a value.
@param b the new b value.
@param c the new c value.
@param alpha the new alpha value.
@param beta the new beta value.
@param gamma the new gamma value.

Sets the cell parameters.
*/
	void SetCell (Lattice lattice, double a, double b, double c, double alpha, double beta, double gamma);

/*!
@param xmin where to store the minimum x.
@param xmax where to store the maximum x.
@param ymin where to store the minimum y.
@param ymax where to store the maximum y.
@param zmin where to store the minimum z.
@param zmax where to store the maximum z.

Retrieves the visible volume in cell coordinates.
*/
	void GetSize (double* xmin, double* xmax, double* ymin, double* ymax, double* zmin, double* zmax);

/*!
@param xmin the new minimum x.
@param xmax the new maximum x.
@param ymin the new minimum y.
@param ymax the new maximum y.
@param zmin the new minimum z.
@param zmax the new maximum z.

Sets the visible volume in cell coordinates.
*/
	void SetSize (double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);

/*!
Checks the atoms list and remove duplicates if needed.
*/
	void CheckAtoms ();

/*!
Checks the cleavages list and remove duplicates if needed.
*/
	void CheckCleavages ();

/*!
Checks the lines list and remove duplicates if needed.
*/
	void CheckLines ();

/*!
@param nPage the dialog id.

Shows the dialog corresponding to \a nPage:
0: cell and lattice dialog,
1: atoms dialog,
2: lines dialog,
3: size dialog,
4: cleavages dialog.
*/
	void Define (unsigned nPage = 0);

/*!
@param pView a new view.

Adds a new view to the document.
*/
	void AddView(View* pView);

/*!
@param pView the view to remove.

	 Removes a view from the document when possible.
@return true if the view could be removed.
*/
	bool RemoveView(View* pView);

/*!
Removes all views from the document. This should be done only when destroying
the document.
*/
	void RemoveAllViews ();

/*!
@return the active view.
*/
	View *GetActiveView () {return m_pActiveView;}

/*!
@return the list of all views.
*/
	std::list <gcr::View *> *GetViews () {return &m_Views;}

/*!
Updates the views window titles.
*/
	void RenameViews ();

/*!
Checks if the document needs to be saved and ask the user about what to do when
needed.

@return tue unless the user cancelled the dialog.
*/
	bool VerifySaved();

/*!
@param widget the new current widget.

Sets the currently active widget.
*/
	void SetWidget (GtkWidget* widget) {m_widget = widget;}

/*!
@return the current document file name.
*/
	char const *GetFileName () {return m_filename;}

/*!
@param pView the new active view.

Sets the new active view.
*/
	void SetActiveView (View *pView) {m_pActiveView = pView;}

/*!
@param filename a file name
@param type an image type such as "png" or "jpeg".
@param options the options to use if any.

Saves the scene as a bitmap.
*/
	void SaveAsImage (const std::string &filename, char const *type, std::map<std::string, std::string>& options);

/*!
@param filename a new file name

Changes the file name.
*/
	void SetFileName (const std::string &filename);

/*!
@param title the new title.

Changes the document title.
*/
	void SetTitle (char const *title);
	void SetTitle (std::string& title);

/*!
@return the current document title.
*/
	char const *GetTitle () {return m_Title.c_str ();}

/*!
@param author the file author's name

Sets the author's name.
*/
	void SetAuthor (char const *author);

/*!
@param mail the file author's mail address

Sets the author's mail address.
*/
	void SetMail (char const *mail);

/*!
@param comment a comment

Adds a comment to the document. Currently only one comment is allowed per
document.
*/
	void SetComment (char const *comment);

/*!
@param label the new window title.

Sets the label to use as window title for views.
*/
	void SetLabel (char const *label);

/*!
@return the document creation date.
*/
	GDate *GetCreationDate () {return &m_CreationDate;}

/*!
@return the document last change date.
*/
	GDate *GetRevisionDate () {return &m_RevisionDate;}

/*!
@return the label to use as window title for views.
*/
	char const *GetLabel () {return m_Label? m_Label: m_DefaultLabel.c_str ();}

/*!
@param FileName a file name.

Saves the scene as a VRML file.
*/
	void OnExportVRML (const std::string &FileName) const;

/*!
Saves the document.
*/
	void Save () const;

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
	void Error(int num) const;

protected:
/*!
The Bravais lattice of the crystal.
*/
	Lattice m_lattice;
/*!
The a parameter of the unit cell.
*/
	double m_a;
/*!
The b parameter of the unit cell.
*/
	double m_b;
/*!
The c parameter of the unit cell.
*/
	double m_c;
/*!
The alpha angle of the unit cell.
*/
	double m_alpha;
/*!
The beta angle of the unit cell.
*/
	double m_beta;
/*!
The gamma angle of the unit cell.
*/
	double m_gamma;
/*!
The minimum x coordinate in the representation of the crystal structure.
*/
	double m_xmin;
/*!
The minimum y coordinate in the representation of the crystal structure.
*/
	double m_ymin;
/*!
The minimum z coordinate in the representation of the crystal structure.
*/
	double m_zmin;
/*!
The maximum x coordinate in the representation of the crystal structure.
*/
	double m_xmax;
/*!
The maximum y coordinate in the representation of the crystal structure.
*/
	double m_ymax;
/*!
The maximum z coordinate in the representation of the crystal structure.
*/
	double m_zmax;
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

/*!
The document creation date.
*/
	GDate m_CreationDate;

/*!
The document last revision date.
*/
	GDate m_RevisionDate;

private:
	char *m_filename;
	bool m_bClosing;
	GtkWidget* m_widget;
	View *m_pActiveView;
	std::string m_DefaultLabel;
	char *m_Label;

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
/*!\fn SetSpaceGroup(SpaceGroup const *val)
@param val a SpaceGroup.
Associates a the space group with the lattice.
*/
/*!\fn GetSpaceGroup()
@return the space group associated with the lattice.
*/
/*!\fn GetRefSpaceGroup()
@return the space group associated with the lattice as a reference.
*/
GCU_PROP (gcu::SpaceGroup const *, SpaceGroup)
/*!\fn SetAutoSpaceGroup(bool val)
@param val whether the lattice SpaceGroup should be automatically searched for.

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

/*!\fn SetFixedSize(bool val)
@param val whether cleavages should not change positions in the view.

If true, adding cleavages will not change atoms positions, otherwise, the scene
will be made as large as possible.
*/
/*!\fn GetFixedSize()
@return true if cleavages will not change atoms positions.
*/
/*!\fn GetRefFixedSize()
@return whether cleavages will not change atoms positions as a reference.
*/
GCU_PROP (bool, FixedSize);

/*!\fn SetReadOnly(bool val)
@param val whether the file can be modified.

If true, the document can't be saved using the original file name.
*/
/*!\fn GetReadOnly()
@return whether the file can be modified.
*/
/*!\fn GetRefReadOnly()
@return whether the file can be modified as a reference.
*/
GCU_PROP (bool, ReadOnly)

/*!\var m_Author
The document author's name.
*/
/*!\fn GetReadOnly()
@return the document author's name.
*/
GCU_PROT_POINTER_PROP (char, Author)

/*!\var m_Mail
The document author's mail address.
*/
/*!\fn GetMail()
@return the document author's mail address.
*/
GCU_PROT_POINTER_PROP (char, Mail)

/*!\var m_Comment
The document comment.
*/
/*!\fn GetComment()
@return the document comment.
*/
GCU_PROT_POINTER_PROP (char, Comment)
};

/*!
A table of the Bravais lattices names.
*/
extern char const *LatticeName[];

} // namespace gcr

#endif // GCR_DOCUMENT_H
