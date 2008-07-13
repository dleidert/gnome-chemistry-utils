// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/document.h 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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


#ifndef GCU_DOCUMENT_H
#define GCU_DOCUMENT_H

#include <gcu/object.h>
#include <gcu/dialog-owner.h>
#include <gcu/macros.h>
#include <string>
#include <set>

namespace gcu
{

class Application;
class Dialog;

/*!\class Document gcu/document.h
This class is the base document class.
*/
class Document: public Object, virtual public DialogOwner
{
friend class Object;
friend class Dialog;
public:
/*!
@param App: the Appllcation which owns the new document.

The default constructor. Creates an empty document.
*/
	Document (Application *App = NULL);
/*!
The destructor of Document.
*/
	virtual ~Document ();

public:

/*!
@param id: the original id to tranlate

When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. GetTranslatedId returns
the translated id corresponding to the parameter id.
*/
	std::string& GetTranslatedId (const char* id) {return m_TranslationTable[id];}

/*!
@param Id: the entry to remove

When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. The EraseTranslationTable method removes thenentry correspondig to id.
*/
	void EraseTranslationId (const char* Id) {m_TranslationTable.erase (Id);}

/*!
When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. The EmptyTranslationTable method should be called after pasting to reinitialize the table
to avoid errors on the next paste event.
*/
	void EmptyTranslationTable() {m_TranslationTable.clear();}

/*!
@param title the new document title.
*/
	void SetTitle (std::string& title) {m_Title = title;}
/*!
@param title the new document title.
*/
	void SetTitle (char const *title) {m_Title = title;}
/*!
@return the current document title.
*/
	std::string &GetTitle () {return m_Title;}

/*!
@param pObject an object needing some update.

The gcu::Document class just stores dirty objects, but don't do anything with them. Derived classes
need to implement that, if meaningful.
*/
	void NotifyDirty (Object* pObject) {m_DirtyObjects.insert (pObject);}
/*!
Saves the document. Need to be overloaded by derived class if meaningful. Default
implementation doesn't do anything.
*/
	virtual void Save () const {;}

private:

/*!
@param id: the original id
@param Cache: 

When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. If Chache is set to true GetId adds a new entry in
the table.
GetNewId returns the translated id
*/
	gchar* GetNewId (gchar* id, bool Cache = true);

private:
	std::map <std::string, std::string> m_TranslationTable;//used when Ids translations are necessary (on pasting...)

protected:
/*!
The document title.
*/
	std::string m_Title;

/*!
		 The set of dirty objects, see gcu::Document::NotifyDirty.
*/
	std::set<Object*> m_DirtyObjects;

/*!\var m_App
The Application instance owning the document.
*/
/*!\fn GetApp()
@return a pointer to the Appication instance owning the ocument or NULL for
an orphan document.
*/
GCU_PROT_PROP (Application *, App)
/*!\var m_Empty
Tells if the document is empty or not.
*/
/*!\fn GetEmpty()
@return true if the document does not contain anything, false otherwise.
*/
GCU_PROT_PROP (bool, Empty);
/*!\var m_Scale
The scale factor to be used in IO operations. all coordinates should be multiplied by this factor
when loading and divided when saving. Default value is 1.
*/
/*!\fn GetScale()
@return the current scale factor.
*/
GCU_PROT_PROP (double, Scale);
};


}
#endif	//GCU_DOCUMENT_H
