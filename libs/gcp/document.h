// -*- C++ -*-

/* 
 * GChemPaint library
 * document.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <list>
#include <map>
#include <set>
#include <string>
#include <libxml/tree.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-print.h>
#include <gcu/document.h>
#include <gcu/macros.h>
#include "atom.h"
#include "fragment.h"
#include "bond.h"
#include "molecule.h"
#include "operation.h"

namespace OpenBabel
{
	class OBMol;
}

using namespace OpenBabel;
using namespace std;

namespace gcp {

extern SignalId OnChangedSignal;
extern SignalId OnDeleteSignal;
extern SignalId OnThemeChangedSignal;

class View;
class Application;
class Window;
class Theme;

class Document: public gcu::Document
{
	//Constructor and destructor
public:
	Document (Application *App, bool StandAlone, Window *window = NULL);
	virtual ~Document ();
	
	//Interface
public:
	GtkWidget* GetWidget ();
	View* GetView () {return m_pView;}
	void BuildBondList (list<Bond*>& BondList, Object* obj);
	bool ImportOB (OBMol& Mol);
	void ExportOB ();
	void BuildAtomTable (map<string, unsigned>& AtomTable, Object* obj, unsigned& index);
	void Save ();
	virtual bool Load (xmlNodePtr);
	const gchar* GetTitle ();
	void SetTitle (const gchar* title);
	void SetLabel (const gchar* label);
	const gchar* GetLabel ();
	void SetFileName (string const &, const gchar *mime_type);
	const gchar* GetFileName () {return m_filename;}
	void Print (GnomePrintContext *pc, gdouble width, gdouble height);
	void AddObject (Object* pObject);
	void AddAtom (Atom* pAtom);
	void AddFragment (Fragment* pFragment);
	void AddBond (Bond* pBond);
	void ParseXMLTree (xmlDocPtr xml);
	void LoadObjects (xmlNodePtr node);
	xmlDocPtr BuildXMLTree ();
	void NotifyDirty (Bond* pBond) {m_DirtyObjects.insert (pBond);}
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
	void ExportImage (string const &filename, const char* type, int resolution = -1);
	void SetReadOnly (bool ro);
	bool GetReadOnly () {return m_bReadOnly;}
	virtual double GetYAlign ();
	Window *GetWindow () {return m_Window;}
	void SetTheme (Theme *theme);
	bool OnSignal (SignalId Signal, Object *Child);
	void SetDirty (bool isDirty = true);
	void OnThemeNamesChanged ();
	double GetMedianBondLength ();

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
	set<Object*> m_DirtyObjects;
	bool m_bIsLoading, m_bUndoRedo, m_bReadOnly;
	string m_FileType;
	bool m_bWriteable;
	GDate CreationDate, RevisionDate;
	list<Operation*> m_UndoList, m_RedoList;
	Operation* m_pCurOp;
	Application* m_pApp;
	Window *m_Window;
	unsigned long m_OpID; // last operation ID
	unsigned m_LastStackSize; // undo list size when last saved

/* Theme is not really a read only property, but we provide a special Set
method */
GCU_RO_PROP (Theme*, Theme)
GCU_PROP (double, BondLength)
GCU_PROP (double, BondAngle)
GCU_PROP (double, ArrowLength)
GCU_PROP (gchar*, TextFontFamily)
GCU_PROP (PangoStyle, TextFontStyle)
GCU_PROP (PangoWeight, TextFontWeight)
GCU_PROP (PangoVariant, TextFontVariant)
GCU_PROP (PangoStretch, TextFontStretch)
GCU_PROP (gint, TextFontSize)
GCU_RO_PROP (PangoAttrList*, PangoAttrList)
};

extern list<Document*> Docs;
extern bool bCloseAll;

}	//	namespace gcp

#endif // GCHEMPAINT_DOCUMENT_H
