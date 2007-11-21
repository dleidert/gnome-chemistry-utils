// -*- C++ -*-

/* 
 * GChemPaint library
 * plugin.h 
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

#ifndef GCHEMPAINT_PLUGIN
#define GCHEMPAINT_PLUGIN

#include <set>

namespace gcp {

class Application;

class Plugin
{
public:
	Plugin ();
	virtual ~Plugin ();

	static void LoadPlugins ();
	virtual void Populate (Application* App);

protected:
	virtual void AddRules ();
};

extern std::set<Plugin*> Plugins;

}	//	namespace gcp

#endif //GCHEMPAINT_PLUGIN
