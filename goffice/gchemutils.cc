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
#include "gogchem3dapp.h"
#include <gsf/gsf-impl-utils.h>
#include <goffice/app/module-plugin-defs.h>
#include <goffice/component/go-component-factory.h>
#include <glib/gi18n-lib.h>
#include <map>
#include <string>
#include <cstring>

//gcuGOfficeApplication *app;

extern "C"
{

static struct {
	gcu::ContentType type;
	char const *name;
} content_types[gcu::ContentTypeInvalid] = {
	{gcu::ContentTypeUnknown, "auto"},
	{gcu::ContentType3D, "3d"},
	{gcu::ContentType2D, "2d"},
	{gcu::ContentTypeCrystal, "crystal"},
	{gcu::ContentTypeSpectrum, "spectrum"},
	{gcu::ContentTypeMisc, "misc"}
};

static gcu::ContentType
gcu_content_type_from_str (char const *name)
{
	unsigned i;
	gcu::ContentType ret = gcu::ContentTypeUnknown;

	for (i = 0; i < gcu::ContentTypeInvalid; i++) {
		if (strcmp (content_types[i].name, name) == 0) {
			ret = content_types[i].type;
			break;
		}
	}
	return ret;
}

char const *
gcu_content_type_as_string (gcu::ContentType type)
{
	unsigned i;
	char const *ret = "auto";

	for (i = 0; i < gcu::ContentTypeInvalid; i++) {
		if (content_types[i].type == type) {
			ret = content_types[i].name;
			break;
		}
	}
	return ret;
}

extern GOPluginModuleDepend const go_plugin_depends [] = {
    { "goffice", GOFFICE_API_VERSION }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

static GObjectClass *gogcu_parent_klass;

enum {
	GOGCU_PROP_0,
	GOGCU_PROP_TYPE,
	GOGCU_PROP_PSI,
	GOGCU_PROP_THETA,
	GOGCU_PROP_PHI,
	GOGCU_PROP_3DMODE
};

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
		if (gogcu->type == gcu::ContentTypeUnknown) {
			gogcu->application = Apps[component->mime_type];
			gogcu->type = gogcu->application->GetContentType ();
			switch (gogcu->type) {
			case gcu::ContentType2D:
			default:
				component->resizable = false;
				component->snapshot_type = GO_SNAPSHOT_SVG;
				break;
			case gcu::ContentType3D:
			case gcu::ContentTypeCrystal:
				component->resizable = true;
				component->snapshot_type = GO_SNAPSHOT_PNG;
				break;
			}
		} else
			gogcu->application = Apps[gcu_content_type_as_string (gogcu->type)];
		if (!gogcu->application)
			return;
	}
	if (gogcu->document) {
		delete gogcu->document;
		gogcu->document = NULL;
	}
	gogcu->application->ImportDocument (gogcu);
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
/*	if (!strcmp (component->mime_type, "application/x-gcrystal") ||
	    (!strcmp (component->mime_type, "chemical/x-xyz")) {
		component->resizable = true;
		component->snapshot_type = GO_SNAPSHOT_PNG;
	}*/
}

static void
go_gchemutils_component_set_size (GOComponent *component)
{
	if (component->resizable)
		component->ascent = component->height;
}

static void
go_gchemutils_component_set_property (GObject *obj, guint param_id,
		       GValue const *value, GParamSpec *pspec)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (obj);

	switch (param_id) {
	case GOGCU_PROP_TYPE:
		gogcu->type = gcu_content_type_from_str (g_value_get_string (value));
		break;
	case GOGCU_PROP_PSI:
		// FIXME: implement
		break;
	case GOGCU_PROP_THETA:
		// FIXME: implement
		break;
	case GOGCU_PROP_PHI:
		// FIXME: implement
		break;
	case GOGCU_PROP_3DMODE:
		// FIXME: implement
		break;

	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static void
go_gchemutils_component_get_property (GObject *obj, guint param_id,
		       GValue *value, GParamSpec *pspec)
{
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (obj);

	switch (param_id) {
	case GOGCU_PROP_TYPE:
		g_value_set_string (value, gcu_content_type_as_string (gogcu->type));
		break;
	case GOGCU_PROP_PSI:
		// FIXME: return a non default when model is 3d
		g_value_set_double (value, 70.);	       
		break;
	case GOGCU_PROP_THETA:
		// FIXME: return a non default when model is 3d
		g_value_set_double (value, 10.);	       
		break;
	case GOGCU_PROP_PHI:
		// FIXME: return a non default when model is 3d
		g_value_set_double (value, -90.);	       
		break;
	case GOGCU_PROP_3DMODE:
		g_value_set_string (value, "ball&stick");
		// FIXME: implement
		break;

	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		return; /* NOTE : RETURN */
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
	GOGChemUtilsComponent *gogcu = GO_GCHEMUTILS_COMPONENT (component);
	component->resizable = false;
	component->editable = true;
	component->ascent = 1.;
	component->descent = 0.;
	component->width = 1.;
	component->snapshot_type = GO_SNAPSHOT_SVG;
	gogcu->type = gcu::ContentTypeUnknown;
}

static void
go_gchemutils_component_class_init (GOComponentClass *klass)
{
	GObjectClass *obj_klass = (GObjectClass *) klass;
	obj_klass->finalize = go_gchemutils_component_finalize;
	obj_klass->get_property = go_gchemutils_component_get_property;
	obj_klass->set_property = go_gchemutils_component_set_property;

	gogcu_parent_klass = (GObjectClass*) g_type_class_peek_parent (klass);

	klass->get_data = go_gchemutils_component_get_data;
	klass->set_data = go_gchemutils_component_set_data;
	klass->render = go_gchemutils_component_render;
	klass->edit = go_gchemutils_component_edit;
	klass->mime_type_set = go_gchemutils_component_mime_type_set;
	klass->set_size = go_gchemutils_component_set_size;

	g_object_class_install_property (obj_klass, GOGCU_PROP_TYPE,
					 g_param_spec_string ("type", _("Type"),
							    _("Whether the model should be represented in 2d, 3d, or as a crystal cell"),
							    "auto", static_cast < GParamFlags > (G_PARAM_READWRITE | GO_PARAM_PERSISTENT)));
	g_object_class_install_property (obj_klass, 	GOGCU_PROP_PSI,
					 g_param_spec_double ("psi", _("Psi"),
							    _("Value of Euler's Ψ angle"),
							    -180., 180., 70, static_cast < GParamFlags > (G_PARAM_READWRITE | GO_PARAM_PERSISTENT)));
	g_object_class_install_property (obj_klass, 	GOGCU_PROP_THETA,
					 g_param_spec_double ("theta", _("Theta"),
							    _("Value of Euler's Θ angle"),
							    0., 180., 10, static_cast < GParamFlags > (G_PARAM_READWRITE | GO_PARAM_PERSISTENT)));
	g_object_class_install_property (obj_klass, 	GOGCU_PROP_PHI,
					 g_param_spec_double ("phi", _("Phi"),
							    _("Value of Euler's Φ angle"),
							    -180., 180., -90., static_cast < GParamFlags > (G_PARAM_READWRITE | GO_PARAM_PERSISTENT)));
	g_object_class_install_property (obj_klass, GOGCU_PROP_TYPE,
					 g_param_spec_string ("mode", _("Display mode"),
							    _("The display mode for the molecule: \"ball&stick\", \"spacefill\", \"cylinders\", or \"wireframe\""),
							    "ball&stick", static_cast < GParamFlags > (G_PARAM_READWRITE | GO_PARAM_PERSISTENT)));
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
	go_components_set_mime_suffix ("chemical/x-xyz", "*.xyz");
	go_components_set_mime_suffix ("application/x-gchempaint", "*.gchempaint");
	go_components_set_mime_suffix ("application/x-gcrystal", "*.gcrystal");
	Apps["2d"] = Apps["application/x-gchempaint"] = new GOGcpApplication ();
	Apps["crystal"] = Apps["application/x-gcrystal"] = new GOGCrystalApplication ();
	Apps["3d"] = Apps["chemical/x-xyz"] = new GOGChem3dApplication ();
// TODO: add other types
}

G_MODULE_EXPORT void
go_plugin_shutdown (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	// TODO: clean
}

}	// extern "C"
