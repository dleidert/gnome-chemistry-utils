// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.cc 
 *
 * Copyright (C) 2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include <gconf/gconf-client.h>
#include <sys/stat.h>

using namespace gcu;

Application::Application (string name)
{
	Name = name;
	string lang = getenv ("LANG");
	HelpFilename = string (DATADIR"/gnome/help/") + Name + string ("/") + lang + string ("/") + Name + ".xml";
	GConfClient* cli = gconf_client_get_default ();
	if (cli) {
		const char *value;
		GConfEntry* entry = gconf_client_get_entry (cli, "/desktop/gnome/applications/help_viewer/exec", NULL, true, NULL);
		if (entry) {
			value = gconf_value_get_string (gconf_entry_get_value (entry));
			if (value) HelpBrowser = value;
		}
	}
}

Application::~Application ()
{
}

void Application::OnHelp (string& tag)
{
	if (!HasHelp ())
		return;
	char *argv[3] = {NULL, NULL, NULL};
	argv[0] = (char*) HelpBrowser.c_str();
	string path = HelpFilename;
	if (tag.length ())
		path += string("#") + Name + string ("-") + tag;
	argv[1] = (char*) path.c_str ();
	g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
		NULL, NULL, NULL, NULL);
}

bool Application::HasHelp ()
{
	if (!HelpBrowser.length () || !HelpFilename.length ())
		return false;
	struct stat buf;
	gint err;
	err = stat(HelpFilename.c_str (), &buf);
	return (err)? false: true;	
}
