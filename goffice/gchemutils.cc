/* 
 * GChemUtils GOffice component
 * gchemutils.cc
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gchemutils-priv.h"
#include "gogcpapp.h"

#include <gsf/gsf-impl-utils.h>
#include <goffice/app/module-plugin-defs.h>
#include <goffice/component/go-component-factory.h>
#include <openbabel/mol.h>
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
	bool result = false;
	return result;
}

static void
go_gchemutils_component_set_data (GOComponent *component)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
}
	
static void
go_gchemutils_component_render (GOComponent *component, cairo_t *cr,
						  double width, double height)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
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
go_gchemutils_component_finalize (GObject *obj)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (obj);
	G_OBJECT_CLASS (gogcu_parent_klass)->finalize (obj);
}

static void
go_gchemutils_component_init (GOComponent *component)
{
	component->resizable = false;
	component->editable = true;
	component->window = NULL;
	component->ascent = 1.;
	component->descent = 0.;
	component->width = 1.;
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
}

GSF_DYNAMIC_CLASS (GOGChemUtilsComponent, go_gchemutils_component,
	go_gchemutils_component_class_init, go_gchemutils_component_init,
	GO_COMPONENT_TYPE)

/*************************************************************************************/

G_MODULE_EXPORT void
go_plugin_init (GOPlugin *plugin, GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
#ifdef ENABLE_NLS
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif
	GTypeModule *module = go_plugin_get_type_module (plugin);
	go_gchemutils_component_register_type (module);
//	go_components_set_mime_suffix ("chemical/x-xyz", "*.xyz");
	go_components_set_mime_suffix ("application/x-gchempaint", "*.gchempaint");
	Apps["application/x-gchempaint"] = new GOGcpApplication ();
//	go_components_set_mime_suffix ("application/x-gcrystal", "*.gcrystal");
// TODO: add other types
}

G_MODULE_EXPORT void
go_plugin_shutdown (GOPlugin *plugin, GOCmdContext *cc)
{
	// TODO: clean
}

}	// extern "C"
