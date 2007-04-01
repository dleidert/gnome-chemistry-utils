// -*- C++ -*-

/* 
 * GChemPaint bonds plugin
 * delocalizedtool.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "delocalizedtool.h"

gcpDelocalizedTool::gcpDelocalizedTool (gcp::Application *App): gcp::Tool (App, "DelocalizedBond")
{
}

gcpDelocalizedTool::~gcpDelocalizedTool ()
{
}

bool gcpDelocalizedTool::OnClicked ()
{
	return false;
}

void gcpDelocalizedTool::OnDrag ()
{
}

void gcpDelocalizedTool::OnRelease ()
{
}
