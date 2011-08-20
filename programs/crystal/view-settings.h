// -*- C++ -*-

/*
 * Gnome Crystal
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

#ifndef GCRYSTAL_VIEW_SETTINGS_H
#define GCRYSTAL_VIEW_SETTINGS_H

#include "view.h"
#include <gcugtk/dialog.h>

class gcViewSettingsDlg: public gcugtk::Dialog
{
friend class gcViewSettingsDlgPrivate;
public:
	gcViewSettingsDlg (gcView* pView);
	virtual ~gcViewSettingsDlg ();

private:
	gcView *m_pView;
	GtkColorButton *Background;
	GtkSpinButton *FoV;
	GtkEntry *Psi, *Theta, *Phi;
	unsigned long PsiSignal, ThetaSignal, PhiSignal;
};

#endif //GCRYSTAL_VIEW_SETTINGS_H
