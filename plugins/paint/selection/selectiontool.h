// -*- C++ -*-

/*
 * GChemPaint selection plugin
 * selectiontool.h
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

#ifndef GCHEMPAINT_SELECTION_TOOL_H
#define GCHEMPAINT_SELECTION_TOOL_H

#include <gcp/tool.h>
#include <list>
#include <gcu/object.h>

namespace gcugtk {
class UIManager;
}

namespace gcp {
	class WidgetData;
	class Application;
}

class gcpSelectionTool: public gcp::Tool
{
public:
	gcpSelectionTool (gcp::Application *App);
	virtual ~gcpSelectionTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	void Activate ();
	bool Deactivate ();
	bool OnRightButtonClicked (gcu::UIManager *UIManager);
	GtkWidget *GetPropertyPage ();
	bool CopySelection (G_GNUC_UNUSED GtkClipboard *clipboard) {return false;} // allow clipboard operations

	void AddSelection (gcp::WidgetData* data);
	void OnFlip (bool horizontal);
	void Rotate (bool rotate);
	void Merge ();

	void CreateGroup ();
	void Group ();
	char const *GetHelpTag ();

	static void OnWidgetDestroyed (GtkWidget *widget, gcpSelectionTool *tool);

private:
	std::map <gcp::WidgetData *, guint> SelectedWidgets;
	bool m_bRotate;
	double m_cx, m_cy;
	double m_dAngle, m_dAngleInit;
	gcp::Operation *m_pOp;
	gcu::TypeId m_Type;
	std::list<int> m_uiIds;
	gcugtk::UIManager *m_UIManager;
	GtkWidget *m_MergeBtn;
};

#endif // GCHEMPAINT_SELECTION_TOOL_H
