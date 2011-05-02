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

/*!\file*/

namespace gcp {

class Application;

/*!\class Plugin gcp/plugin.h
\brief Class for GChemPaint plugins.

Every plugin should implement a new Plugin class derived from this one and
create a unique static instance of the new class. The base class constructor
will register the plugin.
Such plugins are loaded on program startup.
*/
class Plugin
{
public:
/*!
The default constructor. Adds the new plugin to gcp::Plugins.
*/
	Plugin ();
/*!
The destructor.
*/
	virtual ~Plugin ();

/*!
Loads plugins from the GChemPaint plugin directory stored in the PLUGINSDIR
variable.
*/
	static void LoadPlugins ();

/*!
Unloads plugins.
*/
	static void UnloadPlugins ();

/*!
@param App the GChemPaint application.

Called by the framework so that the plugin can add new UI elements to the
application.
*/
	virtual void Populate (Application* App);

/*!
Called by the framework so that the plugin can clean memory before exit
*/
	virtual void Clear ();
};

/*!\var Plugins
The set of registered plugins.
*/
extern std::set<Plugin*> Plugins;

}	//	namespace gcp

#endif //GCHEMPAINT_PLUGIN
