// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/dialog.h 
 *
 * Copyright (C) 2001-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_DIALOG_H
#define GCU_DIALOG_H
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <string>

using namespace std;

namespace gcu {

enum CheckType
{
	NoCheck,
	Min,
	Max,
	MinMax,
	MinEq,
	MaxEq,
	MinEqMax,
	MinMaxEq,
	MinEqMaxEq
};

class Application;
	
class Dialog
{
public:
	Dialog (Application* App, const char* filename, const char* windowname, void (*extra_destroy)(gpointer) = NULL, gpointer data = NULL);
	virtual ~Dialog ();

	virtual void Destroy ();
	virtual bool Apply ();
	void Help ();
	GtkWindow* GetWindow () {return dialog;}

protected:
	bool GetNumber (GtkEntry *Entry, double *x, CheckType c = NoCheck, double min = 0, double max = 0);

	void (*m_extra_destroy) (gpointer);
	gpointer m_data;
	GladeXML* xml;
	char m_buf[64];
	string m_windowname;
	GtkWindow *dialog;
	Application *m_App;
};

}	// namespace gcu

#endif // GCU_DIALOG_H
