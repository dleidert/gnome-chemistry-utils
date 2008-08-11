// -*- C++ -*-

/* 
 * GChemPaint library
 * document.h 
 *
 * Copyright (C) 2001-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_DOCUMENT_H
#define GCHEMPAINT_DOCUMENT_H

#include "operation.h"
#include <gcu/document.h>
#include <gcu/macros.h>
#include <gcu/printable.h>
#include <gcu/residue.h>
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
class Document: public gcu::Document, public gcu::Printable
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
	GtkWidget* GetWidget ();
/*!
@return the gcp::View associated with the document.
*/
	View* GetView () {return m_pView;}
/*!
@param

*/
	void BuildBondList (std::list<Bond*>& BondList, gcu::Object const *obj) const;
	bool ImportOB (OpenBabel::OBMol& Mol);
/*!

*/
	void ExportOB () const;
	void BuildAtomTable (std::map<std::string, unsigned>& AtomTable, gcu::Object const *obj, unsigned& index) const;
	void Save () const;
	virtual bool Load (xmlNodePtr);
	const gchar* GetTitle () const;
	void SetTitle (const gchar* title);
	void SetLabel (const gchar* label);
	const gchar* GetLabel () const;
	void SetFileName (std::string const &, const gchar *mime_type);
	const gchar* GetFileName () {return m_filename;}
	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context) const;
	void AddObject (Object* pObject);
	void AddAtom (Atom* pAtom);
	void AddFragment (Fragment* pFragment);
	void AddBond (Bond* pBond);
	void ParseXMLTree (xmlDocPtr xml);
	void LoadObjects (xmlNodePtr node);
	xmlDocPtr BuildXMLTree () const;
	void Update ();
	void Remove (Object*);
	void Remove (const char* Id);
	void OnProperties ();
	void OnUndo ();
	void OnRedo ();
	const GDate* GetCreationDate () {return &CreationDate;}
	const GDate* GetRevisionDate () {return &RevisionDate;}
	const gchar* GetAuthor () {return m_author;}
	const gchar* GetMail () {return m_mail;}
	const gchar* GetComment () {return m_comment;}
	void SetAuthor (const gchar* author);
	void SetMail (const gchar* mail);
	void SetComment (const gchar* comment);
	void FinishOperation ();
	void AbortOperation ();
	void PopOperation ();
	void PushOperation (Operation* operation, bool undo = true);
	void SetActive ();
	Operation* GetNewOperation (OperationType type);
	Operation* GetCurrentOperation () {return m_pCurOp;}
	void AddData (xmlNodePtr node);
	bool CanUndo () {return m_UndoList.size() > 0;}
	void SetEditable (bool editable) {m_bWriteable = editable; m_bUndoRedo = true;}
	bool GetEditable () {return m_bWriteable;}
	gcp::Application* GetApplication () {return m_pApp;}
	void ExportImage (std::string const &filename, const char* type, int resolution = -1);
	void SetReadOnly (bool ro);
	bool GetReadOnly () {return m_bReadOnly;}
	virtual double GetYAlign ();
	Window *GetWindow () {return m_Window;}
	GtkWindow *GetGtkWindow ();
	void SetTheme (Theme *theme);
	bool OnSignal (gcu::SignalId Signal, gcu::Object *Child);
	void SetDirty (bool isDirty = true);
	void OnThemeNamesChanged ();
	double GetMedianBondLength ();
	bool SetProperty (unsigned property, char const *value);
	void SetLoading (bool loading) {m_bIsLoading = loading;}
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

private:
	void RemoveAtom (Atom* pAtom);
	void RemoveBond (Bond* pBond);
	void RemoveFragment (Fragment* pFragment);

	//Implementation
private:
	View * m_pView;
	gchar* m_filename;
	gchar *m_title;
	gchar *m_label;
	gchar *m_comment, *m_author, *m_mail;
	bool m_bIsLoading, m_bUndoRedo, m_bReadOnly;
	std::string m_FileType;
	bool m_bWriteable;
	GDate CreationDate, RevisionDate;
	std::list<Operation*> m_UndoList, m_RedoList;
	Operation* m_pCurOp;
	Application* m_pApp;
	Window *m_Window;
	unsigned long m_OpID; // last operation ID
	unsigned m_LastStackSize; // undo list size when last saved
	std::set<Residue const *> m_SavedResidues;
	std::map<std::string, gcu::SymbolResidue> m_Residues;

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
/*!\fn SetTextFontFamily(gchar* val)
@param val the new text font family.

Sets the new current text font family for the document.
*/
/*!\fn GetTextFontFamily()
@return the current text font family.
*/
/*!\fn GetRefTextFontFamily()
@return the current text font family as a reference.
*/
GCU_PROP (gchar*, TextFontFamily)
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
/*!\fn SetTextFontSize(gint val)
@param val the new text font size.

Sets the new current text font size for the document.
*/
/*!\fn GetTextFontSize()
@return the current text font size.
*/
/*!\fn GetRefTextFontSize()
@return the current text font size as a reference.
*/
GCU_PROP (gint, TextFontSize)
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
};

extern std::list<Document*> Docs;
extern bool bCloseAll;

}	//	namespace gcp

#endif // GCHEMPAINT_DOCUMENT_H
