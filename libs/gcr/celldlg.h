// -*- C++ -*-

/*
 * Gnome Crystal
 * celldlg.h
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCR_CELLDLG_H
#define GCR_CELLDLG_H

#include <gcugtk/dialog.h>

/*!\file*/

namespace gcr {

class Document;
class Application;

/*!\class CellDlg gcr/celldlg.h
\brief GCrystal cell dialog class.

This class wraps the dialog used to define the crystal cell.
*/
class CellDlg: public gcugtk::Dialog
{
friend class CellDlgPrivate;
public:
/*!
@param App the application running the dialog.
@param pDoc the document.

Creates the dialog.
*/
	CellDlg (Application *App, Document* pDoc);
/*!
The destructor.
*/
	virtual ~CellDlg ();

private:
	char m_buf[64];
	Document *m_pDoc;
	gdouble m_a, m_b, m_c, m_alpha, m_beta, m_gamma;
	GtkComboBox* TypeMenu;
	GtkEntry *A, *B, *C, *Alpha, *Beta, *Gamma;
	GtkToggleButton *AutoSpaceGroup;
	GtkSpinButton *SpaceGroup;
	GtkAdjustment *SpaceGroupAdj;
	unsigned SpaceGroupSignal, TypeSignal,
			 ASignal, BSignal, CSignal,
			 AlphaSignal, BetaSignal, GammaSignal;
};

}	//	namespace gcr

#endif //GCR_CELLDLG_H
