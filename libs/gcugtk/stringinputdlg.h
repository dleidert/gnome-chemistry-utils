// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/stringinputdlg.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_GTK_STRING_INPUT_DLG_H
#define GCU_GTK_STRING_INPUT_DLG_H

#include <gcugtk/dialog.h>
#include <string>

/*!\file*/

namespace gcu {
	class Document;
}

namespace gcugtk {

typedef void (*StringInputCB) (gcu::Document *doc, char const *str);

/*!\class StringInputDlg gcugtk/stringinputdlg.h
Represents the dialog used to enter a string. It is used by GChemPaint and
GChem3d to import an InChI or a SMILES.
*/
class StringInputDlg: public gcugtk::Dialog
{
public:
	StringInputDlg (gcu::Document *doc, StringInputCB cb, char const *title);
	virtual ~StringInputDlg ();

	bool Apply ();

private:
	gcu::Document *m_Doc;
	StringInputCB m_CB;
};

}	//	namespace gcugtk

#endif //	GCU_GTK_STRING_INPUT_DLG_H
