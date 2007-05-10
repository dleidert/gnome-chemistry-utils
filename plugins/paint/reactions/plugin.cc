// -*- C++ -*-

/* 
 * GChemPaint reactions plugin
 * plugin.cc 
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcp/application.h>
#include <gcp/document.h>
#include <glib/gi18n-lib.h>

gcpReactionsPlugin plugin;

gcpReactionsPlugin::gcpReactionsPlugin (): gcp::Plugin ()
{
}

gcpReactionsPlugin::~gcpReactionsPlugin ()
{
}

struct CallbackData {
	Object *arrow;
	Object *child;
};

static void do_attach_object ()
{
}

static void do_free_data (struct CallbackData *data)
{
	delete data;
}

static bool on_reaction_arrow_menu (Object *target, GtkUIManager *UIManager, Object *object, double x, double y)
{
	gcp::Document *Doc = dynamic_cast<gcp::Document*> (target->GetDocument ());
	gcp::WidgetData* pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (Doc->GetWidget ()), "data");
	if (pData->SelectedObjects.size () != 1)
		return target->Object::BuildContextualMenu (UIManager, object, x, y);
	Object *obj = pData->SelectedObjects.front ();
	TypeId Id = obj->GetType ();
	if ((Id != MoleculeType && Id != TextType) || obj->GetGroup ())
		return target->Object::BuildContextualMenu (UIManager, object, x, y);
	GtkActionGroup *group = gtk_action_group_new ("reaction-arrow");
	GtkAction *action = gtk_action_new ("Arrow", _("Arrow"), NULL, NULL);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	struct CallbackData *data = new struct CallbackData ();
	data->arrow = target;
	data->child = obj;
	action = gtk_action_new ("attach", _("Attach selection to arrow..."), NULL, NULL);
	g_object_set_data_full (G_OBJECT (action), "data", data, (GDestroyNotify) do_free_data);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (do_attach_object), data);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Arrow'><menuitem action='attach'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	return true;
}

void gcpReactionsPlugin::Populate (gcp::Application *App)
{
	Object::AddMenuCallback (ReactionArrowType, on_reaction_arrow_menu);
}
