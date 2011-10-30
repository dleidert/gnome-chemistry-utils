/*
 * GChemUtils GOffice component
 * gchemutils.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "gchemutils-priv.h"
#include "gogcpapp.h"
#include "gogcrystalapp.h"
#include <gsf/gsf-impl-utils.h>
#include <goffice/app/module-plugin-defs.h>
#include <goffice/component/go-component-factory.h>
#include <libintl.h>
#include <map>
#include <string>
#include <cstring>

//gcuGOfficeApplication *app;

extern "C"
{

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

static GObjectClass *gogcu_parent_klass;

using namespace std;
static map <string, GOGcuApplication *> Apps;

static gboolean
go_gchemutils_component_get_data (GOComponent *component, gpointer *data, int *length,
									void (**clearfunc) (gpointer), gpointer *user_data)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
	return gogcu->application->GetData (gogcu, data, length, clearfunc, user_data);
}

static void
go_gchemutils_component_set_data (GOComponent *component)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
	if (!gogcu->application) {
		gogcu->application = Apps[component->mime_type];
		if (!gogcu->application)
			return;
	}
	gogcu->document = gogcu->application->ImportDocument (component->mime_type, component->data, component->length);
	gogcu->application->UpdateBounds (gogcu);
}

static void
go_gchemutils_component_render (GOComponent *component, cairo_t *cr,
						  double width, double height)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
	gogcu->application->Render (gogcu, cr, width, height);
}

static GtkWindow*
go_gchemutils_component_edit (GOComponent *component)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
	if (!gogcu->document) {
		component->ascent = 1.;
		component->descent = 0.;
		component->width = 1.;
	}
	if (gogcu->window) {
		gogcu->window->Show ();
		return gogcu->window->GetWindow ();
	}
	if (!gogcu->application) {
		gogcu->application = Apps[component->mime_type];
		if (!gogcu->application)
			return NULL;
	}
	return gogcu->application->EditDocument (gogcu);
}

static void
go_gchemutils_component_mime_type_set (GOComponent *component)
{
	if (!strcmp (component->mime_type, "application/x-gcrystal")) {
		component->resizable = true;
		component->snapshot_type = GO_SNAPSHOT_PNG;
	}
}

static void
go_gchemutils_component_finalize (GObject *obj)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (obj);
	if (gogcu->window)
		gogcu->window->Destroy ();
	G_OBJECT_CLASS (gogcu_parent_klass)->finalize (obj);
}

static void
go_gchemutils_component_init (GOComponent *component)
{
	component->resizable = false;
	component->editable = true;
	component->ascent = 1.;
	component->descent = 0.;
	component->width = 1.;
	component->snapshot_type = GO_SNAPSHOT_SVG;
}

static void
go_gchemutils_component_class_init (GOComponentClass *klass)
{
	GObjectClass *obj_klass = (GObjectClass *) klass;
	obj_klass->finalize = go_gchemutils_component_finalize;

	gogcu_parent_klass = (GObjectClass*) g_type_class_peek_parent (klass);

	klass->get_data = go_gchemutils_component_get_data;
	klass->set_data = go_gchemutils_component_set_data;
	klass->render = go_gchemutils_component_render;
	klass->edit = go_gchemutils_component_edit;
	klass->mime_type_set = go_gchemutils_component_mime_type_set;
}

GSF_DYNAMIC_CLASS (GOGChemUtilsComponent, go_gchemutils_component,
	go_gchemutils_component_class_init, go_gchemutils_component_init,
	GO_TYPE_COMPONENT)

/*************************************************************************************/

G_MODULE_EXPORT void
go_plugin_init (GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	GTypeModule *module = go_plugin_get_type_module (plugin);
	go_gchemutils_component_register_type (module);
//	go_components_set_mime_suffix ("chemical/x-xyz", "*.xyz");
	go_components_set_mime_suffix ("application/x-gchempaint", "*.gchempaint");
	go_components_set_mime_suffix ("application/x-gcrystal", "*.gcrystal");
	Apps["application/x-gchempaint"] = new GOGcpApplication ();
	Apps["application/x-gcrystal"] = new GOGCrystalApplication ();
// TODO: add other types
}

G_MODULE_EXPORT void
go_plugin_shutdown (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	// TODO: clean
}

}	// extern "C"
