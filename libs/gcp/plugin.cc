// -*- C++ -*-

/* 
 * GChemPaint library
 * plugin.cc 
 *
 * Copyright (C) 2004 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "plugin.h"
#include "application.h"
#include <dlfcn.h>
#include <glib.h>

using namespace std;

namespace gcp {

set<Plugin*> Plugins;

Plugin::Plugin ()
{
	Plugins.insert (this);
}

Plugin::~Plugin ()
{
}

void Plugin::LoadPlugins ()
{
	GDir* dir = g_dir_open (PLUGINSDIR, 0, NULL);
	if (dir ==  NULL)
		return;
	const char* name;
	while ((name = g_dir_read_name (dir))) {
		if ((strcmp (name + strlen (name) - 3, ".so")))
			continue;
		name = g_strconcat (PLUGINSDIR"/", name, NULL);
		if (!dlopen (name, RTLD_NOW))
			puts (dlerror());
		g_free ((void*) name);
	}
	g_dir_close (dir);
	set<Plugin*>::iterator i = Plugins.begin (), end = Plugins.end ();
	while (i != end)
		(*i++)->AddRules ();
}

void Plugin::Populate (Application* App)
{
}

void Plugin::AddRules ()
{
}

}
