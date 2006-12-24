// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/document.h 
 *
 * Copyright (C) 2004-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcu/macros.h>
#include <string>

using namespace std;

namespace gcu
{

class Application;

/*!\class Document gcu/document.h
This class is the base document class.
*/
class Document: public Object
{
friend class gcu::Object;
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
	string& GetTranslatedId (const char* id) {return m_TranslationTable[id];}

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
	void SetTitle (string& title) {m_Title = title;}
/*!
@param title the new document title.
*/
	void SetTitle (char const *title) {m_Title = title;}
/*!
@return the current document title.
*/
	string &GetTitle () {return m_Title;}

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
	map <string, string> m_TranslationTable;//used when Ids translations are necessary (on pasting...)

protected:
/*!
The document title.
*/
	string m_Title;

/*!\var m_App
The Application instance owning the document.
*/
/*!\fn GetApp()
@return a pointer to the Appication instance owning the ocument or NULL for
an orphan document.
*/
GCU_PROT_PROP (Application *, App)
/*!\fn SetDirty(bool dirty)
@param dirty should be true if the document has changed, false otherwise.
*/
/*!\fn GetDirty()
@return true if the document has changed since it was opened or last saved,
false otherwise.
*/
/*!\fn GetRefDirty()
*@return the current state of the document as a reference:
true if the document has changed since it was opened or last saved, false otherwise.
*/
GCU_PROP (bool, Dirty);
/*!\var m_Empty
Tells if the document is empty or not.
*/
/*!\fn GetEmpty()
@return true if the document does not contain anything, false otherwise.
*/
GCU_PROT_PROP (bool, Empty);
};


}
#endif	//GCU_DOCUMENT_H
