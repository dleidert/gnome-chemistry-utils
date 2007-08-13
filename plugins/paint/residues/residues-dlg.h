// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * residues-dlg.h
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_RESIDUE_DLG_H
#define GCP_RESIDUE_DLG_H

#include <gcu/dialog.h>
#include <gcu/macros.h>
#include <gcp/target.h>

class gcpPseudoAtom;
using namespace std;
using namespace gcu;

namespace gcp {
class Application;
class Document;
}

class gcpResiduesDlg: public Dialog, public gcp::Target
{
public:
	gcpResiduesDlg (gcp::Application *App);
	virtual ~gcpResiduesDlg ();
	
	void Add ();
	void Remove ();
	bool Close ();

	bool OnKeyPress (GdkEventKey *event);
	bool OnKeyRelease (GdkEventKey *event);
	void OnCurChanged (int num);
	void OnSave ();
	void OnDelete ();
	void OnSymbolActivate ();
	void OnNameActivate ();

private:
	gcpPseudoAtom *m_Atom;
	GtkComboBox *m_CurBox;
	GtkWidget *m_SaveBtn, *m_DeleteBtn;
	GtkEntry *m_SymbolEntry, *m_NameEntry;
	bool m_ValidName, m_ValidSymbols;

GCU_PROP (int, Page);
GCU_PROP (bool, Generic);
};

#endif	//	GCP_RESIDUE_DLG_H
