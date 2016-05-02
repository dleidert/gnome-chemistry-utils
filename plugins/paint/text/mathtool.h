// -*- C++ -*-

/*
 * GChemPaint text plugin
 * mathtool.h
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

#ifndef GCHEMPAINT_MATH_TOOL_H
#define GCHEMPAINT_MATH_TOOL_H

#include <gcp/tool.h>

class gcpMathTool: public gcp::Tool
{
public:
	gcpMathTool (gcp::Application *App);
	virtual ~gcpMathTool ();

	bool OnClicked ();
};

#endif	// GCHEMPAINT_MATH_TOOL_H
