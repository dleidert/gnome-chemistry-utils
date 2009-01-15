// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * curvedarrowtool.h
 *
 * Copyright (C) 2004-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_CURVED_ARROW_TOOL_H
#define GCHEMPAINT_CURVED_ARROW_TOOL_H

#include <gcp/tool.h>

class gcpCurvedArrowTool: public gcp::Tool
{
public:
	gcpCurvedArrowTool (gcp::Application *App, std::string Id);
	virtual ~gcpCurvedArrowTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnMotion ();
	void OnRelease ();

private:
	bool m_Full; // if false use half heads
};

#endif	//GCHEMPAINT_CURVED_ARROW_TOOL_H
