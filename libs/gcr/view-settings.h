// -*- C++ -*-

/*
 * Gnome Crystal library
 * view-settings.h
 *
 * Copyright (C) 2001-2011 Jean Br�fort <jean.brefort@normalesup.org>
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

#ifndef GCR_VIEW_SETTINGS_H
#define GCR_VIEW_SETTINGS_H

#include <gcugtk/dialog.h>

namespace gcr {

class View;

class ViewSettingsDlg: public gcugtk::Dialog
{
friend class ViewSettingsDlgPrivate;
public:
	ViewSettingsDlg (View* pView);
	virtual ~ViewSettingsDlg ();

private:
	View *m_pView;
	GtkColorButton *Background;
	GtkSpinButton *FoV;
	GtkEntry *Psi, *Theta, *Phi;
	unsigned long PsiSignal, ThetaSignal, PhiSignal;
};

}	//	namespace gcr

#endif //	GCR_VIEW_SETTINGS_H