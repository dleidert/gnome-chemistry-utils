// -*- C++ -*-

/* 
 * GChemPaint library
 * H-pos.h
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

#ifndef GCHEMPAINT_H_POS_DLG_H
#define GCHEMPAINT_H_POS_DLG_H

#include <gcu/dialog.h>
#include <gtk/gtkcombobox.h>

namespace gcp {

class Atom;
class Document;
class View;

class HPosDlg: public gcu::Dialog
{
public:
	HPosDlg (Document *pDoc, Atom* pAtom);
	virtual ~HPosDlg ();
	
	void OnPosChanged ();
	
private:
	GtkComboBox *box;
	Atom *m_Atom;
	View *m_View;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_H_POS_DLG_H
