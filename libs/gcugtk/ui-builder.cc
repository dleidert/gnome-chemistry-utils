// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/ui-builder.cc
 *
 * Copyright (C) 2009-2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "ui-builder.h"
#include <glib/gi18n-lib.h>
#include <string>

namespace gcugtk
{

UIBuilder::UIBuilder (): gcu::UIBuilder ()
{
	m_Builder = gtk_builder_new ();
}

UIBuilder::UIBuilder (char const *filename, char const *domain) throw (std::runtime_error):
	gcu::UIBuilder ()
{
	m_Builder = go_gtk_builder_load (filename, domain, NULL);
	if (!m_Builder) {
		char *buf = g_strdup_printf (_("Could not load %s."), filename);
		std::string mess = buf;
		g_free (buf);
		throw std::runtime_error (mess);
	}
}

UIBuilder::~UIBuilder ()
{
	if (m_Builder)
		g_object_unref (m_Builder);
}

GtkWidget *UIBuilder::GetWidget (char const *wname)
{
	GObject *obj = gtk_builder_get_object (m_Builder, wname);
	return (obj)? GTK_WIDGET (obj): NULL;
}

GtkWidget *UIBuilder::GetRefdWidget (char const *wname)
{
	// this code is copied from a goffice patch, replace it when goffice has it
	GObject *obj = gtk_builder_get_object (m_Builder, wname);
	if (obj) {
		g_object_ref (obj);
		return GTK_WIDGET (obj);
	} else
		return NULL;
}

GObject *UIBuilder::GetObject (char const *name)
{
	return gtk_builder_get_object (m_Builder, name);
}

void UIBuilder::ActivateActionWidget (char const *path, bool activate)
{
	GtkWidget *w = go_gtk_builder_get_widget (m_Builder, path);
	if (w)
		gtk_widget_set_sensitive (w, activate);
}

}   //  namespace gcugtk
