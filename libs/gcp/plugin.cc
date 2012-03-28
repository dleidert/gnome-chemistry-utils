// -*- C++ -*-

/*
 * GChemPaint library
 * plugin.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "plugin.h"
#include "application.h"
#include <dlfcn.h>
#include <glib.h>
#include <cstring>

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
}

void Plugin::UnloadPlugins ()
{
	set<Plugin*>::iterator i, end = Plugins.end ();
	for (i = Plugins.begin (); i != end; i++)
		(*i)->Clear ();
}

void Plugin::Populate (G_GNUC_UNUSED Application* App)
{
}

 void Plugin::Clear ()
 {
 }

}
