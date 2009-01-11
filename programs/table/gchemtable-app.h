// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-app.h 
 *
 * Copyright (C) 2005-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMTABLE_APP_H
#define GCHEMTABLE_APP_H

#include <gcu/application.h>
#include <gcu/dialog.h>
#include <gcu/gtkperiodic.h>
#include <map>

using namespace gcu;

class GChemTableCurve;

class GChemTableApp: public Application
{
public:
	GChemTableApp ();
	virtual ~GChemTableApp ();

	void OnElement (int Z);
	GtkWindow *GetWindow () {return GTK_WINDOW (window);}
	void OnAbout ();
	void ClearPage (int Z);
	void SetCurZ (int Z);
	void SetColorScheme (char const *name);
	void SetTemperature (double T);
 	void SetFamily (int family_N);
	void GetStateColor (int Z, GdkColor *color);
	void GetFamilyColor (int Z, GdkColor *color);
	void GetAcidityColor (int Z, GdkColor *color);
	void GetElectronegColor (int Z, GdkColor *color);
	void GetRadiusColor (int Z, GdkColor *color);
	void GetBlockColor (int Z, GdkColor *color);
	void OnNewChart ();
	void OnSaveAsImage (GChemTableCurve *curve);
	bool FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *Doc);
	

private:
	Dialog *Pages[118];
	GtkWidget *window;
	GtkPeriodic *periodic;
	int m_CurZ;
	std::map <std::string, unsigned> colorschemes;
	double temperature;
 	int family;
};

#endif	// GCHEMTABLE_APP_H
