// -*- C++ -*-

/*
 * GChemPaint library
 * stringdlg.h
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_STRING_DLG_H
#define GCHEMPAINT_STRING_DLG_H

#include <gcu/dialog.h>
#include <string>

/*!\file*/
namespace gcp {

class Document;

/*!\class StringDlg gcp/stringdlg.h
Represents the dialog used to display the InChI or canonical SMILES of a molecule.
*/
class StringDlg: public gcu::Dialog
{
public:
/*!\enum data_type
The string types supported by StringDlg. 
*/
	enum data_type {
/*!
SMILES representation of a molecule.
*/
		SMILES,
/*!
InChI of a molecule.
*/
		INCHI
	};
/*!
@param pDoc the document owning the dialog.
@param data the string to display.
@param type the type of the data.

Builds a dialog showing the string and display it. \a type is used to select
an appropriate title for the dialog box.
*/
	StringDlg (Document *pDoc, std::string& data, enum data_type type);
/*!
The destructor.
*/
	virtual ~StringDlg ();
	
/*!
Called by the framework to save the string to a file upon user request.
*/
	bool Apply ();
/*!
Called by the framework when the user clicks the "Copy" button.
*/
	void Copy ();
/*!
@param clipboard the clipboard requesting the data.
@param selection_data the GtkSelectionData to which the string should be copied.
@param info the type of data requested. Unused since only ASCII strings are
supported.

Called by the framework to get the actual data for the clipboard.
*/
	void OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info);

private:
	enum data_type Type;
	std::string Data;
	GtkTextView *View;
	GtkTextBuffer *Buffer;
};

}	//	namespace gcp

#endif //GCHEMPAINT_STRING_DLG_H
