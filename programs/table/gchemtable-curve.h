// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-curve.h 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMTABLE_CURVE_H
#define GCHEMTABLE_CURVE_H

#include <gcu/dialog.h>
#include <gcu/printable.h>
#include <string>
#include "gchemtable-app.h"
#include <goffice/graph/gog-graph.h>
#include <gtk/gtkpagesetup.h>
#include <gtk/gtkprintsettings.h>

using namespace gcu;

class GChemTableCurve: public Dialog, public Printable
{
public:
	GChemTableCurve (GChemTableApp *App, char const *name);
	virtual ~GChemTableCurve ();

	GChemTableApp *GetApplication () {return dynamic_cast <GChemTableApp *> (m_App);}

	void OnPageSetup ();
	void OnCopy ();
	void OnClose ();
	void OnProperties ();

	void DoPrint (GtkPrintOperation *print, GtkPrintContext *context, int page) const;
	void SetGraph (GogGraph *graph);
	GtkWindow *GetGtkWindow () {return GTK_WINDOW (dialog);}
	void ClearGuru () {m_Guru = NULL;}
	void SaveAsImage (std::string const &filename, char const *mime_type, unsigned width, unsigned height) const;

private:
	std::string m_Name;
	GogGraph *m_Graph;
	GtkWidget *m_GraphWidget, *m_GraphBox, *m_Guru;
};

#endif	// GCHEMTABLE_CURVE_H
