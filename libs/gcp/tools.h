// -*- C++ -*-

/* 
 * GChemPaint
 * tools.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TOOLS_H
#define GCHEMPAINT_TOOLS_H

#include <gcu/dialog.h>
#include <map>

using namespace std;
using namespace gcu;

namespace gcp {

class Tools: public Dialog
{
public:
	Tools (Application *App);
	virtual ~Tools ();

	void Show (bool visible);
	void AddToolbar (string &name);
	void SetUIManager (GtkUIManager *manager);
	void SetPage (Tool *tool, int i);
	void OnSelectTool (Tool *tool);
	void RegisterTool (GtkWidget *w);
	void OnElementChanged (int Z);
	void Update (void);
	void OnHelp ();

private:
	GtkUIManager *m_UIManager;
	GtkBox *m_ButtonsBox;
	map<Tool*, int> m_Pages;
	GtkNotebook *m_Book;
	Tool *m_Tool;
};

}	// namespace gcp

#endif // GCHEMPAINT_TOOLS_H
