// -*- C++ -*-

/*
 * Gnome Crystal
 * atomsdlg.h
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#ifndef GCR_ATOMSDLG_H
#define GCR_ATOMSDLG_H

#include <gcugtk/dialog.h>
#include <gcugtk/gcuperiodic.h>
#include "grid.h"
#include <vector>

using namespace std;

namespace gcr {
class Document;
class Application;

class AtomsDlg: public gcugtk::Dialog
{
friend class AtomsDlgPrivate;
public:
	AtomsDlg (Application *App, Document* pDoc);
	virtual ~AtomsDlg ();

	void Closed ();
	void ReloadData ();

private:
	void PopulateRadiiMenu ();

private:
	Document *m_pDoc;
	GcrGrid *m_Grid;
	GcuPeriodic* periodic;
	GtkToggleButton* CustomColor;
	GtkColorButton *AtomColor;
	GtkEntry *AtomR;
	unsigned short m_nElt;
	std::vector < Atom * > m_Atoms;
	int m_AtomSelected;
	GtkWidget *DeleteBtn, *DeleteAllBtn, *SelectEltBtn;
	GtkComboBoxText *RadiusTypeMenu, *RadiusMenu;
	GtkSpinButton *ChargeBtn, *ScaleBtn;
	const GcuAtomicRadius **m_Radii;
	GcuAtomicRadius m_Radius;
	int m_RadiusType, m_Charge;
	vector<int> m_RadiiIndex;
	unsigned long m_RadiiSignalID, m_EntryFocusOutSignalID, m_ColorSignalID,
				  m_RadiusTypeSignalID, m_ChargeSignalID, m_ScaleSignalID;
	bool m_Closing;
	double m_Ratio;
};

}	//	namespace gcr

#endif //GCR_ATOMSDLG_H
