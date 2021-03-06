// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * loopwtool.h
 *
 * Copyright (C) 2014 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_LOOP_TOOL_H
#define GCHEMPAINT_LOOP_TOOL_H

#include <gcp/tool.h>
#include <gcu/object.h>
#include <gccv/structs.h>
#include <map>

class gcpLoopTool: public gcp::Tool
{
public:
	gcpLoopTool (gcp::Application* App);
	~gcpLoopTool ();

	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	void OnMotion ();

private:
	bool clockwise;
	std::map < gcu::Object*, gccv::Rect > steps;
};

#endif  //  GCHEMPAINT_LOOP_TOOL_H
