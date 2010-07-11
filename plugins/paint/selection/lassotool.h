// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * lassotool.h 
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_LASSO_TOOL_H
#define GCHEMPAINT_LASSO_TOOL_H

#include <gcp/tool.h>
#include <map>

class gcpLassoTool: public gcp::Tool
{
public:
	gcpLassoTool (gcp::Application *App);
	virtual ~gcpLassoTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	void Activate ();
	bool Deactivate ();
	bool CopySelection (G_GNUC_UNUSED GtkClipboard *clipboard) {return false;} // allow clipboard operations

	void AddSelection (gcp::WidgetData* data);

	static void OnWidgetDestroyed (GtkWidget *widget, gcpLassoTool *tool);

private:
	std::map <gcp::WidgetData *, guint> SelectedWidgets;
};

#endif // GCHEMPAINT_LASSO_TOOL_H
