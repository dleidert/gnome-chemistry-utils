// -*- C++ -*-

/* 
 * GChemPaint Wikipedia plugin
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
#include <gcp/tool.h>
#include "wikipediatool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpWikipediaPlugin plugin;

gcpWikipediaPlugin::gcpWikipediaPlugin (): gcp::Plugin ()
{
}

gcpWikipediaPlugin::~gcpWikipediaPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Wikipedia", favicon},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"Wikipedia", "gcp_Wikipedia", N_("Wikipedia export"), NULL,
		N_("Export for Wikipedia publication"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'/>"
"	 <placeholder name='Select2'/>"
"	 <placeholder name='Select3'>"
"	   <separator/>"
"	   <toolitem action='Wikipedia'/>"
"	 </placeholder>"
"  </toolbar>"
"</ui>";

struct CallbackData {
	Object *Mol;
	double x, y;
};

static void do_image_export (struct CallbackData *data)
{
	gcp::Document *Doc = dynamic_cast <gcp::Document*> (data->Mol->GetDocument ());
	if (!Doc)
		return;
	gcp::Application *App = dynamic_cast <gcp::Application*> (Doc->GetApp ());
	gcp::Tool *Tool = App->GetTool ("Wikipedia");
	Tool->OnClicked (Doc->GetView (), data->Mol, data->x, data->y, 0);
}

static void do_free_data (struct CallbackData *data)
{
	delete data;
}

static bool on_molecule_menu (Object *target, GtkUIManager *UIManager, Object *object, double x, double y)
{
	gcp::Document *Doc = dynamic_cast <gcp::Document*> (target->GetDocument ());
	if (!Doc)
		return false;
	GtkActionGroup *group = gtk_action_group_new ("wikipedia");
	struct CallbackData *data = new struct CallbackData ();
	data->Mol = target;
	data->x = x;
	data->y = y;
	GtkAction *action = gtk_action_new ("wikipedia", _("Generate Wikipedia conformant PNG image"), NULL, NULL);
	g_object_set_data_full (G_OBJECT (action), "data", data, (GDestroyNotify) do_free_data);
	g_signal_connect_swapped (action, "activate", G_CALLBACK (do_image_export), data);
	gtk_action_group_add_action (group, action);
	g_object_unref (action);
	gtk_ui_manager_add_ui_from_string (UIManager, "<ui><popup><menu action='Molecule'><menuitem action='wikipedia'/></menu></popup></ui>", -1, NULL);
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
	return true;
}

void gcpWikipediaPlugin::Populate (gcp::Application *App)
{
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	Object::AddMenuCallback (MoleculeType, on_molecule_menu);
	new gcpWikipediaTool (App);
}
