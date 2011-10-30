/*
 * Gnome Chemistry Utils
 * gcugtk/ui-manager.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "ui-manager.h"

namespace gcugtk {

UIManager::UIManager (GtkUIManager *ui):
	gcu::UIManager (),
	m_UIManager (ui)
{
}

UIManager::~UIManager ()
{
}

void UIManager::ActivateActionWidget (char const *path, bool activate)
{
	GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, path);
	if (w)
		gtk_widget_set_sensitive (w, activate);
}

}	//	namespace gcugtk