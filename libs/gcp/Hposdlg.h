// -*- C++ -*-

/* 
 * GChemPaint library
 * H-pos.h
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_H_POS_DLG_H
#define GCHEMPAINT_H_POS_DLG_H

#include <gcugtk/dialog.h>

/*!\file*/
namespace gcp {

class Atom;
class Document;
class View;

/*!\class HPosDlg gcp/Hposdlg.h
The dialog used to set the position of the symbols of hydrogen atoms attached
to heteroatoms or explicit carbon atoms.
*/
class HPosDlg: public gcugtk::Dialog
{
public:
/*!
@param pDoc the document containing the atom.
@param pAtom the atom with attached hydrogens

Creates and open a dialog used to set the position of the symbols of hydrogen
atoms attached to the specified atom.
*/
	HPosDlg (Document *pDoc, Atom* pAtom);
/*!
The destructor.
*/
	virtual ~HPosDlg ();

/*!
Called by the framework when the selected position has changed.
*/
	void OnPosChanged ();

private:
	GtkComboBox *box;
	Atom *m_Atom;
	View *m_View;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_H_POS_DLG_H
