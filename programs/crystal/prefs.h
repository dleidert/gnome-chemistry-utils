// -*- C++ -*-

/* 
 * Gnome Crystal
 * prefs.h 
 *
 * Copyright (C) 2001-2006 Jean Br�fort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GCRYSTAL_PREFS_H
#define GCRYSTAL_PREFS_H

#include <gcu/dialog.h>

using namespace gcu;

class gcApplication;

class gcPrefsDlg: public Dialog
{
public:
	gcPrefsDlg (gcApplication *App);
	virtual ~gcPrefsDlg ();

	virtual bool Apply ();
	void UpdatePrinting ();

private:
	GtkComboBox *PrintResMenu;
	GtkSpinButton *PrintResBtn;
	GtkColorButton *BackgroundBtn;
	GtkSpinButton *FoVBtn;
	GtkEntry *PsiEnt, *ThetaEnt, *PhiEnt;;
};

#endif //GCRYSTAL_PREFS_H