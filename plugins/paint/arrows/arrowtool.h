// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * arrowtool.h 
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

#ifndef GCHEMPAINT_ARROW_TOOL_H
#define GCHEMPAINT_ARROW_TOOL_H

#include <gcp/tool.h>
#include <gcp/arrow.h>

enum {
	gcpDoubleHeadedArrow = gcp::FullReversibleArrow + 1,
	gcpDoubleQueuedArrow
};

class gcpArrowTool: public gcp::Tool
{
public:
	gcpArrowTool (gcp::Application* App, unsigned ArrowType = gcp::SimpleArrow);
	~gcpArrowTool ();
	
	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	void Activate ();
	void SetArrowType (unsigned ArrowType) {m_ArrowType = ArrowType;}
	GtkWidget *GetPropertyPage ();
	char const *GetHelpTag () {return "arrow";}
	void SetLength (double length);

protected:
	GnomeCanvasPoints *points;

private:
	double m_dAngle;
	unsigned m_ArrowType;
	GtkSpinButton *m_LengthBtn;
};

#endif	//GCHEMPAINT_ARROW_TOOL_H
