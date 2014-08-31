// -*- C++ -*-

/*
 * Gnome Crystal
 * linesdlg.h
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_LINESDLG_H
#define GCR_LINESDLG_H

#include <gcugtk/dialog.h>
#include "grid.h"
#include <vector>

/*!\file*/
namespace gcr {

class Document;
class Application;
class Line;

/*!\class LinesDlg gcr/linesdlg.h
\brief GCrystal lines dialog class.

This class wraps the dialog used to define lines inside a crystal representation.
*/
class LinesDlg: public gcugtk::Dialog
{
friend class LinesDlgPrivate;
public:
/*!
@param App the application running the dialog.
@param pDoc the document.

Creates the dialog.
*/
	LinesDlg (Application *App, Document* pDoc);
/*!
The destructor.
*/
	virtual ~LinesDlg ();

/*!
Reloads the lines list from the document. The list might have changed after
simplification following duplicates detection.
*/
	void ReloadData ();

private:
	void Closed ();

private:
	Document *m_pDoc;
	GtkColorChooser *LineColor, *EdgesColor, *MediansColor, *DiagsColor;
	GtkEntry *LineR, *EdgesR, *MediansR, *DiagsR;
	GtkCheckButton *EdgesBtn, *MediansBtn, *DiagsBtn;
	GtkWidget *DeleteBtn, *DeleteAllBtn;
	int m_LineSelected;
	GcrGrid *m_Grid;
	std::vector < Line * > m_Lines;
	Line *Edges, *Diagonals, *Medians;
	bool m_Closing;
	unsigned long m_EdgesFocusOutSignalID, m_DiagsFocusOutSignalID,
		m_MediansFocusOutSignalID, m_LineFocusOutSignalID, m_ColorChangedID;
	double m_Radius;
	GdkRGBA m_rgba;
};

}	//	namespace gcr

#endif //GCR_LINESDLG_H
