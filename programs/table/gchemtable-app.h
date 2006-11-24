// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-app.h 
 *
 * Copyright (C) 2005 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMTABLE_APP_H
#define GCHEMTABLE_APP_H

#include <gcu/application.h>
#include <gcu/dialog.h>
#include <gcu/gtkperiodic.h>
#include <map>

using namespace gcu;

class GChemTableApp: public Application
{
public:
	GChemTableApp ();
	virtual ~GChemTableApp ();

	void OnElement (int Z);
	GtkWindow *GetWindow () {return GTK_WINDOW (window);}
	void ClearPage (int Z);
	void SetCurZ (int Z);
	void SetColorScheme (char const *name);
#ifdef WITH_BODR
	void SetTemperature (double T);
	void GetStateColor (int Z, GdkColor *color);
	void GetFamilyColor (int Z, GdkColor *color);
	void GetAcidityColor (int Z, GdkColor *color);
#endif

private:
	Dialog *Pages[118];
	GtkWidget *window;
	GtkPeriodic *periodic;
	int m_CurZ;
	map <string, unsigned> colorschemes;
#ifdef WITH_BODR
	double temperature;
#endif
};

#endif	// GCHEMTABLE_APP_H
