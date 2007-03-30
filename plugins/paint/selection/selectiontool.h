// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * selectiontool.h 
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

#ifndef GCHEMPAINT_SELECTION_TOOL_H
#define GCHEMPAINT_SELECTION_TOOL_H

#include <gcp/tool.h>

class gcp::WidgetData;
class gcp::Application;

class gcpSelectionTool: public gcp::Tool
{
public:
	gcpSelectionTool (gcp::Application *App);
	virtual ~gcpSelectionTool ();

	virtual bool OnClicked ();
	virtual void OnDrag ();
	virtual void OnRelease ();
	virtual void Activate ();
	virtual bool Deactivate ();
	virtual bool OnRightButtonClicked (GtkUIManager *UIManager);
	virtual GtkWidget *GetPropertyPage ();

	void AddSelection (gcp::WidgetData* data);
	void OnFlip (bool horizontal);
	void Rotate (bool rotate);
	void Merge ();

	void CreateGroup ();
	void Group ();
	char const *GetHelpTag ();

private:
	std::list<gcp::WidgetData*> SelectedWidgets;
	bool m_bRotate;
	double m_cx, m_cy;
	double m_dAngle, m_dAngleInit;
	gcp::Operation *m_pOp;
	TypeId m_Type;
	list<int> m_uiIds;
	GtkUIManager *m_UIManager;
	GtkWidget *m_MergeBtn;
};

#endif // GCHEMPAINT_SELECTION_TOOL_H
