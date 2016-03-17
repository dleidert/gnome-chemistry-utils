// -*- C++ -*-

/*
 * GChemPaint library
 * document.h
 *
 * Copyright (C) 2001-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_DOCUMENT_H
#define GCHEMPAINT_DOCUMENT_H

#include "operation.h"
#include <gcu/document.h>
#include <gcu/macros.h>
#include <gcugtk/printable.h>
#include <gcu/residue.h>
#include <gcugtk/printable.h>
#include <list>
#include <map>
#include <set>
#include <string>
#include <libxml/tree.h>
#include <gtk/gtk.h>

/*!\file*/
namespace OpenBabel
{
	class OBMol;
}

namespace gcp {

/*!
Signal emitted when an object has been modified. See
gcu::Object::EmitSignal for more information.
*/
extern gcu::SignalId OnChangedSignal;
/*!
Signal emitted when an object has been deleted. See
gcu::Object::EmitSignal for more information.
*/
extern gcu::SignalId OnDeleteSignal;
/*!
Signal emitted when the theme has changed. This signal is
called by the theme for all its documents.
*/
extern gcu::SignalId OnThemeChangedSignal;

class View;
class Application;
class Window;
class Theme;
class Residue;
class Atom;
class Bond;
class Fragment;
class Molecule;

/*!\class Document gcp/document.h
The document class for GChemPaint.
*/
class Document: public gcu::Document, public gcugtk::Printable
{
	//Constructor and destructor
public:
/*!
@param App the application.
@param StandAlone whether the document is opened in its own window or
embedded in something else.
@param window the document window if it has already been created or NULL.

Constructs a new empty document using default theme.
*/
	Document (Application *App, bool StandAlone, Window *window = NULL);
/*!
The destructor.
*/
	virtual ~Document ();

	//Interface
public:
/*!
Empties a document and reinitialize it.
*/
	void Clear ();
/*!
@return the canvas widget where the document is displayed.
*/
	GtkWidget* GetWidget () const;
/*!
@return the gcp::View associated with the document.
*/
	View *GetView () {return m_pView;}
/*!
@return the gcp::View associated with the document.
*/
	View const*GetView () const {return m_pView;}
/*!
Saves the current file.
*/
	void Save () const;
/*!
@param node the XML root node for the document.

Loads the document from the XML tree representing it.
@return true on success, false otherwise.
*/
	virtual bool Load (xmlNodePtr node);
/*!
@return the document title.
*/
	const char* GetTitle () const;
/*!
@param label the new window title.

Sets the label to use as window title.
*/
	void SetLabel (char const *label);
/*!
@return the window title.
*/
	char const *GetLabel () const;
/*!
@param filename the new file name (URI).
@param mime_type the new mime type.

Sets the new file name and its associated mime type.
*/
	void SetFileName (std::string const &filename, char const *mime_type);
/*!
@return the current file name, actually the URI.
*/
	char const *GetFileName () const {return m_filename;}
/*!
@param print a GtkPrintOperation.
@param	context a GtkPrintContext.
@param page the page to print.

Prints the document.
*/
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const;
/*!
@param pObject a new object.

Adds a new object to the document.
*/
	void AddObject (Object* pObject);
/*!
@param pAtom a new atom.

Adds a new atom to the document.
*/
	void AddAtom (Atom* pAtom);
/*!
@param pFragment a new atoms group.

Adds a new fragment to the document.
*/
	void AddFragment (Fragment* pFragment);
/*!
@param pBond a new bond.

Adds a new bond to the document and checks the connectivity of the new covalent
structure.
*/
	void AddBond (Bond* pBond);
/*!
@param xml the XML document representing the GChemPaint document being loaded.

Parses the XML tree and creates all objects it represents.
*/
	void ParseXMLTree (xmlDocPtr xml);
/*!
@param node the XML node representing objects to add to the document.

*/
	void LoadObjects (xmlNodePtr node);
/*!
Builds the XML tree representing the document. The returned value must
be freed using xmlFree.
@return the new XML document.
*/
	xmlDocPtr BuildXMLTree () const;
/*!
Updates the view for all objects which have been marked as dirty.
*/
	void Update ();
/*!
@param object the object to remove.

Removes a child object from the document, and deletes it.
*/
	void Remove (Object* object);
/*!
@param Id the Id of the object to be removed.

Removes a child object from the document, and deletes it.
*/
	void Remove (const char* Id);
/*!
Called by the framework when the user fires the File/Properties command.
*/
	void OnProperties ();
/*!
Called by the framework when the user fires the Edit/Undo command.
*/
	void OnUndo ();
/*!
Called by the framework when the user fires the Edit/Redo command.
*/
	void OnRedo ();
/*!
@return the date at which the document was first created.
*/
	const GDateTime* GetCreationDate () {return CreationDate;}
/*!
@return the last date at which the document was modified.
*/
	const GDateTime* GetRevisionDate () {return RevisionDate;}
/*!
@return the author's name.
*/
	char const *GetAuthor () {return m_author;}
/*!
@return the author's e-mail address.
*/
	char const *GetMail () {return m_mail;}
/*!
@param author the new author name.

Setes the document author name.
*/
	void SetAuthor (char const *author);
/*!
@param mail the new e-mail address.

Sets the document author e-mail address.
*/
	void SetMail (char const *mail);
/*!
Ends the current operation and pushes it on top of the undo stack. This method
must be called after all changes have been done in  the document and the changes
described in the operation.
*/
	void FinishOperation ();
/*!
Aborts and deletyes the current operation.
*/
	void AbortOperation ();
/*!
Removes an operation from the udo stack and deletes it.
*/
	void PopOperation ();
/*!
@param operation the operation to add.
@param undo whether to put he operation on the undo or the redo stack.

Adds the current operation to the appropriate task.
*/
	void PushOperation (Operation* operation, bool undo = true);
/*!
Called by the framework when the document becomes the active one. Updates the
menus according to the document state.
*/
	void SetActive ();
/*!
@param type the type of the new undo/redo operation.

@return the new operation.
*/
	Operation* GetNewOperation (OperationType type);
/*!
@return the current undo/redo operation.
*/
	Operation* GetCurrentOperation () {return m_pCurOp;}
/*!
@param node the XML node representing objects to add to the document.

Adds previously serialized objects from the clipboard to the document. Links
can be set only to or from pasted objects.
*/
	void PasteData (xmlNodePtr node);
/*!
@return true if the undo stack is not empty.
*/
	bool CanUndo () {return m_UndoList.size() > 0;}
/*!
@param editable whether the document might be edited or not

This method is used to lock a document and inhibit any change in it.
*/
	void SetEditable (bool editable) {m_bWriteable = editable; m_bUndoRedo = true;}
/*!
@return true if the document can be edited.
*/
	bool GetEditable () {return m_bWriteable;}
/*!
@return the application owning the document.
*/
	gcp::Application* GetApplication () {return m_pApp;}
/*!
@param filename the image filename.
@param type a string representing the image type like "png", "svg", or "eps".
@param resolution the resolution to use in the image for bitmaps or −1.

Exports the current document as an image. The image is limited to the real bounds
of the document. For bitmaps, the size is evaluated using the perceived screen resolution.
If the default resolution (−1) is used, the resoution will be the screen resolution.
*/
	void ExportImage (std::string const &filename, const char* type, int resolution = -1);
/*!
@param ro whether the file is read-only or not.

If ro is true, the File/Save menu item and corresponding button will be disabled.
*/
	void SetReadOnly (bool ro);
/*!
@return true if the file is read-only or false if it is writeable.
*/
	bool GetReadOnly () {return m_bReadOnly;}
/*!
Used to retrieve the y coordinate for alignment. If the documetn contains just one
objecst as a molecule or a reaction, it will reurn it's alignment value, otherwise
0 is returned.
@return y coordinate used for alignment.
*/
	double GetYAlign () const;
/*!
@return the gcp::Window displaying the document if any.
*/
	gcu::Window *GetWindow ();
/*!
@return GtkWindow displaying the document if any.
*/
	GtkWindow *GetGtkWindow ();
/*!
@param theme the new theme for the document.

Sets the theme to be used by the document. This will not change lengths of
existing bonds or size of existing text.
*/
	void SetTheme (Theme *theme);
/*!
@param Signal a SignalId
@param Child the child which emitted the signal or NULL

Called when a signal has been emitted by a child. Only OnThemeChangedSignal
is relevant for documents.
@return false since documents have no parent.
*/
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
/*!
Mark the document as dirty. On any attempt to close a dirty document, a
dialog box is opened to ask the user if he wants to save the modified document
or drop the changes.
*/
	void SetDirty (bool isDirty = true);
/*!
Called by the framework when the theme names have changed, i.e. a new theme has
ben added, or a theme has been removed or renamed.
*/
	void OnThemeNamesChanged ();
/*!
@return the median value of bond lengths. This is used when importing data
from a document with a different theme or from an other program to scale the
data so that the bond lengths fit the themed bond length.
*/
	double GetMedianBondLength ();
/*!
@param property the property id as defined in objprops.h
@param value the property value as a string

Used when loading to set properties. Supported properties for documents are:
GCU_PROP_DOC_FILENAME, GCU_PROP_DOC_MIMETYPE, GCU_PROP_DOC_TITLE,
GCU_PROP_DOC_COMMENT, GCU_PROP_DOC_CREATOR, GCU_PROP_DOC_CREATION_TIME,
GCU_PROP_DOC_MODIFICATION_TIME, and GCU_PROP_THEME_BOND_LENGTH.
*/
	bool SetProperty (unsigned property, char const *value);

/*!
@param property the property id as defined in objprops.h

Used when saving to get properties from document.. Supported properties for documents are:
GCU_PROP_DOC_FILENAME, GCU_PROP_DOC_MIMETYPE, GCU_PROP_DOC_TITLE,
GCU_PROP_DOC_COMMENT, GCU_PROP_DOC_CREATOR, GCU_PROP_DOC_CREATION_TIME,
GCU_PROP_DOC_MODIFICATION_TIME, and GCU_PROP_THEME_BOND_LENGTH.
*/
	std::string GetProperty (unsigned property) const;

/*!
@param loading whether the document is loading data or not.

Used to inhibit undo/redo operation creation when loading.
*/
	void SetLoading (bool loading) {m_bIsLoading = loading;}
/*!
@param r the residue to be saved.
@param node the XML node to which add the saved residue if needed.

GChemPaint saves the meaning of a residue the first time it is encountered, and
maintains a list of saved residues to avoid duplicates.
*/
	void SaveResidue (Residue const *r, xmlNodePtr node);
/*!
@param symbol the symbol for which a Residue* is searched.
@param ambiguous where to store the boolean telling if the symbol is ambiguous
or NULL.

Documents might own not global residues with the samesymbol or name
but a different meaning from the standard residue.
@return the Residue* found or NULL.
*/
	gcu::Residue const *GetResidue (char const *symbol, bool *ambiguous = NULL);
/*!
@param name the name of the new residue.
@param symbol the symbol of the new residue.
@param molecule a molecule with a pseudo atom which describes the structure
of the residue.

@return the new Residue on success or NULL.
*/
	gcu::Residue *CreateResidue (char const *name, char const *symbol, gcu::Molecule *molecule);

/*!
@return the set of objects created during the current operation (such as
pasting data). This is used to properly update links such as which bonds
correspond to which atoms as the pasted objects might have the same id than
already existing objects.
*/
	std::set <std::string> &GetNewObjects () {return m_NewObjects;}

private:
	void RemoveAtom (Atom* pAtom);
	void RemoveBond (Bond* pBond);
	void RemoveFragment (Fragment* pFragment);

	//Implementation
private:
	View * m_pView;
	char *m_filename;
	char *m_label;
	char *m_author, *m_mail;
	bool m_bIsLoading, m_bUndoRedo, m_bReadOnly;
	std::string m_FileType;
	bool m_bWriteable;
	GDateTime *CreationDate, *RevisionDate;
	std::list<Operation*> m_UndoList, m_RedoList;
	Operation* m_pCurOp;
	Application* m_pApp;
	Window *m_Window;
	unsigned long m_OpID; // last operation ID
	unsigned m_LastStackSize; // undo list size when last saved
	std::set<Residue const *> m_SavedResidues;
	std::map<std::string, gcu::SymbolResidue> m_Residues;
	std::set <std::string> m_NewObjects;

/* Theme is not really a read only property, but we provide a special Set
method */
/*!\fn GetTheme()
@return the theme used by the document.
*/
GCU_RO_PROP (Theme*, Theme)
/*!\fn SetBondLength(double val)
@param val the new default bond length.

Sets the new default bond length for the document.
*/
/*!\fn GetBondLength()
@return the current default bond length.
*/
/*!\fn GetRefBondLength()
@return the current default bond length as a reference.
*/
GCU_PROP (double, BondLength)
/*!\fn SetBondAngle(double val)
@param val the new default bond angle between two consecutive bonds
in a chain.

Sets the new default bond angle between two consecutive bonds in a chain
for the document.
*/
/*!\fn GetBondAngle()
@return the current default angle between two consecutive bonds in a chain.
*/
/*!\fn GetRefBondAngle()
@return the current default angle between two consecutive bonds in a chain
as a reference.
*/
GCU_PROP (double, BondAngle)
/*!\fn SetArrowLength(double val)
@param val the new default arrow length.

Sets the new default arrow length for the document.
*/
/*!\fn GetArrowLength()
@return the current default arrow length.
*/
/*!\fn GetRefArrowLength()
@return the current default arrow length as a reference.
*/
GCU_PROP (double, ArrowLength)
/*!\fn SetTextFontFamily(char *val)
@param val the new text font family.

Sets the new current text font family for the document.
*/
/*!\fn GetTextFontFamily()
@return the current text font family.
*/
/*!\fn GetRefTextFontFamily()
@return the current text font family as a reference.
*/
GCU_PROP (char*, TextFontFamily)
/*!\fn SetTextFontStyle(PangoStyle val)
@param val the new text font style.

Sets the new current text font style for the document.
*/
/*!\fn GetTextFontStyle()
@return the current text font style.
*/
/*!\fn GetRefTextFontStyle()
@return the current text font style as a reference.
*/
GCU_PROP (PangoStyle, TextFontStyle)
/*!\fn SetTextFontWeight(PangoWeight val)
@param val the new text font weight.

Sets the new current text font weight for the document.
*/
/*!\fn GetTextFontWeight()
@return the current text font weight.
*/
/*!\fn GetRefTextFontWeight()
@return the current text font weight as a reference.
*/
GCU_PROP (PangoWeight, TextFontWeight)
/*!\fn SetTextFontVariant(PangoVariant val)
@param val the new text font variant.

Sets the new current text font variant for the document.
*/
/*!\fn GetTextFontVariant()
@return the current text font variant.
*/
/*!\fn GetRefTextFontVariant()
@return the current text font variant as a reference.
*/
GCU_PROP (PangoVariant, TextFontVariant)
/*!\fn SetTextFontStretch(PangoStretch val)
@param val the new text font stretch.

Sets the new current text font stretch for the document.
*/
/*!\fn GetTextFontStretch()
@return the current text font stretch.
*/
/*!\fn GetRefTextFontStretch()
@return the current text font stretch as a reference.
*/
GCU_PROP (PangoStretch, TextFontStretch)
/*!\fn SetTextFontSize(int val)
@param val the new text font size.

Sets the new current text font size for the document.
*/
/*!\fn GetTextFontSize()
@return the current text font size.
*/
/*!\fn GetRefTextFontSize()
@return the current text font size as a reference.
*/
GCU_PROP (int, TextFontSize)
/*!\fn GetPangoAttrList()
@return a PangoAttrList with all the attributes used for atomic symbols display.
*/
GCU_RO_PROP (PangoAttrList*, PangoAttrList)
/*!\fn SetAllowClipboard(bool val)
@param val whether the document may use the clipboard or no.

Sets the rights for the document clipbard access.
*/
/*!\fn GetAllowClipboard()
@return whether the document may use the clipboard or not.
*/
/*!\fn GetRefAllowClipboard()
@return whether the document may use the clipboard or not as a reference.
*/
GCU_PROP (bool, AllowClipboard)
/*!\fn GetSoftwareVersion()
Retrieves the GChemPaint API number corresponding to the program which saved
the file last time. Might be 0 if the file has never been saved, or if the file
was not saved using GChemPaint. The version is an unsigned decimal number of the
type MMMmmmµµµ, where MMM is the major version, mmm the minor version and µµµ
the micro version. This is used to detect files with old syntax on loading.
@return the GChemPaint API version which last saved the file.
*/
GCU_RO_PROP (unsigned, SoftwareVersion)
/*!\fn SetBracketsFontFamily(char* val)
@param val the new brackets font family.

Sets the new current brackets font family for the document.
*/
/*!\fn GetBracketsFontFamily()
@return the current brackets font family.
*/
/*!\fn GetRefBracketsFontFamily()
@return the current brackets font family as a reference.
*/
GCU_PROP (std::string, BracketsFontFamily)
/*!\fn SetBracketsFontSize(int val)
@param val the new brackets font size.

Sets the new current text font size for the document.
*/
/*!\fn GetBracketsFontSize()
@return the current brackets font size.
*/
/*!\fn GetRefBracketsFontSize()
@return the current brackets font size as a reference.
*/
GCU_PROP (int, BracketsFontSize)

/*!\fn SetUseAtomColors(bool val)
@param val whether to use symbolic colors for the atomic symbols.

When set, atomic symbols will be displayed using the symbolic colors for the
element (when the color is not white, i.e. hydrogen symbol will always be black).
*/
/*!\fn GetUseAtomColors()
@return whether to use symbolic colors for the atomic symbols.
*/
/*!\fn GetRefUseAtomColors()
@return whether to use symbolic colors for the atomic symbols as a reference.
*/
GCU_PROP (bool, UseAtomColors)
};

}	//	namespace gcp

#endif // GCHEMPAINT_DOCUMENT_H
