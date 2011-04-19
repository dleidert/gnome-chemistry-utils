// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcugtk/application.cc 
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "cmd-context-gtk.h"
#include <gcu/object.h>
#include <glib/gi18n-lib.h>

namespace gcugtk {

WindowState Application::DefaultWindowState = NormalWindowState;

class ApplicationPrivate
{
public:
	static void MaximizeWindows ();
	static void FullScreenWindows ();
};

void ApplicationPrivate::MaximizeWindows ()
{
	Application::DefaultWindowState = MaximizedWindowState;
}

void ApplicationPrivate::FullScreenWindows ()
{
	Application::DefaultWindowState = FullScreenWindowState;
}

static GOptionEntry options[] = 
{
  {"full-screen", 'F', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *)ApplicationPrivate::MaximizeWindows, N_("Open new windows full screen"), NULL},
  {"maximize", 'M', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *) ApplicationPrivate::FullScreenWindows, N_("Maximize new windows"), NULL},
  {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

Application::Application (std::string name, std::string datadir, char const *help_name, char const *icon_name, CmdContextGtk *cc):
	gcu::Application (name, datadir, help_name, icon_name, cc)
{
	m_RecentManager = gtk_recent_manager_get_default ();
	RegisterOptions (options);
}

Application::~Application ()
{
}

void Application::CreateDefaultCmdContext ()
{
	if (!m_CmdContext)
		m_CmdContext = new CmdContextGtk (this);
}

}	//	namespace gcugtk
