// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * orbitaltool.h 
 *
 * Copyright (C) 2002-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_ORBITAL_TOOL_H
#define GCHEMPAINT_ORBITAL_TOOL_H

#include <gcp/tool.h>

class gcpOrbitalTool: public gcp::Tool
{
public:
	gcpOrbitalTool(gcp::Application *App);
	virtual ~gcpOrbitalTool();

	bool OnClicked();
	void OnDrag();
	void OnRelease();
	void OnMotion ();
	GtkWidget *GetPropertyPage ();
	char const *GetHelpTag () {return "orbital";}

private:
	double m_Coef;
};

#endif	//GCHEMPAINT_ORBITAL_TOOL_H
