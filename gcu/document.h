// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * chemistry/document.h 
 *
 * Copyright (C) 2004
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


#ifndef GCU_DOCUMENT_H
#define GCU_DOCUMENT_H

#include "object.h"

using namespace std;

namespace gcu
{
/*!\class Atom gcu/atom.h
This class is used to represent atoms.

*/
class Document: public Object
{
public:
/*!
The default constructor. Creates an empty document.
*/
	Document();
/*!
The destructor of Document.
*/
	virtual ~Document();

public:

/*!

*/
	gchar* GetNewId (gchar* id);

/*!

*/
	string& GetTranslatedId (const char* id) {return m_TranslationTable[id];}

/*!

*/
	void EraseTranslationId (const char* Id) {m_TranslationTable.erase (Id);}

/*!
When pasting, objects added to the document might have the same Id as objects already existing. In such cases, the document
maintains a table to update links using Ids as identifiers. The EmptyTranslationTable method should be called then to reinitialize the table
to avoid errors on the next paste event.
*/
	void EmptyTranslationTable() {m_TranslationTable.clear();}

private:
	map <string, string> m_TranslationTable;//used when Ids translations are necessary (on pasting...)
};

}
#endif	//GCU_DOCUMENT_H
