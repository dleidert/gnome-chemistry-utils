// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-curve.h 
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

#ifndef GCHEMTABLE_CURVE_H
#define GCHEMTABLE_CURVE_H

#include <gcu/dialog.h>
#include <string>
#include "gchemtable-app.h"
#include <goffice/graph/gog-graph.h>
#include <gtk/gtkpagesetup.h>
#include <gtk/gtkprintsettings.h>

using namespace gcu;

class GChemTableCurve: public Dialog
{
public:
	GChemTableCurve (GChemTableApp *App, char const *name);
	virtual ~GChemTableCurve ();

	GChemTableApp *GetApplication () {return dynamic_cast <GChemTableApp *> (m_App);}

	void OnPageSetup ();
	void OnPrint (bool preview);
	void OnCopy ();
	void OnClose ();

	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context);

private:
	string m_Name;
	GogGraph *m_Graph;
	GtkPrintSettings *m_PrintSettings;
	GtkPageSetup *m_PageSetup;
};

#endif	// GCHEMTABLE_CURVE_H
