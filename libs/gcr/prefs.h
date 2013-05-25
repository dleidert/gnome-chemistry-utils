// -*- C++ -*-

/*
 * Gnome Crystal
 * prefs.h
 *
 * Copyright (C) 2001-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_PREFS_H
#define GCR_PREFS_H

#include <gcugtk/dialog.h>

/*!\file*/
namespace gcr {

class Application;

/*!\class PrefsDlg gcr/prefs.h
\brief Application preferences dialog class.

This class wraps the dialog used to define the preferences of the application.
*/
class PrefsDlg: public gcugtk::Dialog
{
friend class PrefsDlgPrivate;
public:
/*!
@param App the application.

Creates the dialog.
*/
	PrefsDlg (Application *App);
/*!
The destructor.
*/
	virtual ~PrefsDlg ();

private:
	GtkComboBox *PrintResMenu;
	GtkSpinButton *PrintResBtn;
	GtkColorChooser *BackgroundBtn;
	GtkSpinButton *FoVBtn;
	GtkEntry *PsiEnt, *ThetaEnt, *PhiEnt;
	unsigned long PsiSignal, ThetaSignal, PhiSignal, PrintResChanged;
};

}	//	namespace gcr

#endif //GCR_PREFS_H
