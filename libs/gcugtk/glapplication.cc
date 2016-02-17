// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/glapplication.cc
 *
 * Copyright (C) 2015 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "glapplication.h"
#include <gcu/macros.h>
#include <cstring>

#define GCUGTK_CONF_DIR "gtk"
#define ROOTDIR "/apps/gchemutils/gtk/"

namespace gcugtk {

class GLApplicationPrivate {
public:
	static void OnConfigChanged (GOConfNode *node, char const *name, GLApplication *app);
};

void GLApplicationPrivate::OnConfigChanged (GOConfNode *node, char const *name, GLApplication *app)
{
	GCU_UPDATE_KEY ("direct-rendering", bool, app->m_RenderDirect, {});
}

GLApplication::GLApplication (std::string name, std::string datadir, char const *help_name, char const *icon_name, CmdContextGtk *cc):
	Application (name, datadir, help_name, icon_name, cc)
{
	m_ConfNode = go_conf_get_node (gcu::Application::GetConfDir (), GCUGTK_CONF_DIR);
	GCU_GCONF_GET ("direct-rendering", bool, m_RenderDirect, false)
	m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) GLApplicationPrivate::OnConfigChanged, NULL);
}

GLApplication::~GLApplication ()
{
	go_conf_remove_monitor (m_NotificationId);
	go_conf_free_node (m_ConfNode);
	m_ConfNode = NULL;
}

} // namespace gcugtk
