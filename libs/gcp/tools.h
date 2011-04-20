// -*- C++ -*-

/* 
 * GChemPaint
 * tools.h 
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <gcugtk/dialog.h>
#include <gcugtk/gcucomboperiodic.h>
#include <map>

/*!\file*/

namespace gcugtk {
class UIManager;
}

namespace gcp {

/*!class Tools gcp/tools.h
The GChemPaint tools box. Only one instance should be created.
*/
class Tools: public gcugtk::Dialog
{
public:
/*!
@param App the GChemPaint Application.

Builds the tools box for the application.
*/
	Tools (Application *App);
/*!
The destructor.
*/
	virtual ~Tools ();

/*!
@param visible whether the tools box should be visible.

Shows or hides the tools box.
*/
	void Show (bool visible);
/*!
@param name the new toolbar name.

Addsa new toolbar.
*/
	void AddToolbar (std::string &name);
/*!
@param manager a gcu::UIManager.

Sets the gcugtk::UIManager associated to the user interface of the tools box.
*/
	void SetUIManager (gcugtk::UIManager *manager);
/*!
@param tool the Tool for which an options page has been added.
@param i the rank of the page in the notebook.

Registers the new option box for the tool, so that the widget is created only
once when the tool is first selected.
*/
	void SetPage (Tool *tool, int i);
/*!
@param tool a Tool.

Called by the framwork when the Tool is selected. If the Tool has an option box,
it becomes the active notebook page after being created if necessary.
*/
	void OnSelectTool (Tool *tool);
/*!
@param w the tool widget, generally a button.

Registers the Tool corresponding to the widget. The widget name is used to
retrieve the Tool from the application using gcp::Application::GetTool().
*/
	void RegisterTool (GtkWidget *w);
/*!
@param Z the new current atomic number.

Called by the framework when the current atomic number has changed.
*/
	void OnElementChanged (int Z);
/*!
@param Z the new current atomic number.

Called by the framework when the user choose a new current atomic number
using the Mendeleiev table widget.
*/
	void SetElement (int Z);
/*!
Called by the framework when the Help button is clicked.
*/
	void OnHelp ();

private:
	gcugtk::UIManager *m_UIManager;
	GtkBox *m_ButtonsBox;
	std::map<Tool*, int> m_Pages;
	GtkNotebook *m_Book;
	Tool *m_Tool;
	GcuComboPeriodic *m_Mendeleiev;
};

}	// namespace gcp

#endif // GCHEMPAINT_TOOLS_H
