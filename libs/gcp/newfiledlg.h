// -*- C++ -*-

/* 
 * GChemPaint libray
 * newfiledlg.h 
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

#ifndef GCHEMPAINT_NEW_FILE_DLG_H
#define GCHEMPAINT_NEW_FILE_DLG_H

#include <gcu/dialog.h>
#include <gcu/macros.h>
#include <gcu/object.h>

namespace gcp {

class Application;
class Theme;

class NewFileDlg: public gcu::Dialog, gcu::Object
{
public:
	NewFileDlg (Application *App);
	virtual ~NewFileDlg ();

	bool Apply ();
	void OnThemeNamesChanged ();

private:
	GtkComboBox *m_Box;
	unsigned m_Lines;
	gulong m_ChangedSignal;

GCU_PROP (Theme*, Theme)
};

}	//	namespace gcp

#endif	// GCHEMPAINT_NEW_FILE_DLG_H
