// -*- C++ -*-

/*
 * GChemPaint atoms plugin
 * elementtool.h
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#ifndef GCHEMPAINT_ELEMENT_TOOL_H
#define GCHEMPAINT_ELEMENT_TOOL_H

#include <gcp/tool.h>

class gcpElementTool: public gcp::Tool
{
public:
	gcpElementTool (gcp::Application *App);
	virtual ~gcpElementTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	char const *GetHelpTag () {return "element";}

private:
	double m_x2, m_y2;
};

#endif // GCHEMPAINT_ELEMENT_TOOL_H
