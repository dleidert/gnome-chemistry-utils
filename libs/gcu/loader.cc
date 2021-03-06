// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/loader.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "cmd-context.h"
#include "document.h"
#include "loader.h"
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>
#include <cstring>

using namespace std;

namespace gcu {

static map<string, LoaderStruct> loaders;
static map<string, GOPluginService *> services;

#define GCU_PLUGIN_SERVICE_CHEMICAL_LOADER_TYPE  (plugin_service_chemical_loader_get_type ())
#define GCU_PLUGIN_SERVICE_CHEMICAL_LOADER(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), GCU_PLUGIN_SERVICE_CHEMICAL_LOADER_TYPE, PluginServiceChemicalLoader))
#define IS_GCU_PLUGIN_SERVICE_CHEMICAL_LOADER(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GCU_PLUGIN_SERVICE_CHEMICAL_LOADER_TYPE))

GType plugin_service_chemical_loader_get_type (void);
typedef struct {
	GOPluginServiceSimple	base;

} PluginServiceChemicalLoader;
typedef GOPluginServiceSimpleClass PluginServiceChemicalLoaderClass;

static void
plugin_service_chemical_loader_read_xml (GOPluginService * service, xmlNode * tree,
				    GOErrorInfo ** ret_error)
{
	xmlNode *ptr;

	g_return_if_fail (service->id != NULL);

	for (ptr = tree->xmlChildrenNode; ptr != NULL; ptr = ptr->next)
		if (0 == xmlStrcmp (ptr->name, (xmlChar const *) "mime_type"))
		{
			char *name = (char *) xmlGetProp (ptr, (xmlChar const *) "name");
			if (name) {
				if (loaders.find (name) != loaders.end ()) {
					*ret_error = go_error_info_new_printf ("Duplicate loader for mime type %s", name);
					xmlFree (name);
					return;
				}
				LoaderStruct l;
				l.read = l.write = l.supports2D = l.supports3D = l.supportsCrystals = l.supportsSpectra = false;
				l.loader = NULL;
				char *prop = (char *) xmlGetProp (ptr, (xmlChar const *) "capabilities");
				if (prop) {
					if (strchr (prop, 'r') != NULL)
						l.read = true;
					if (strchr (prop, 'w') != NULL)
						l.write = true;
					xmlFree (prop);
				}
				prop = (char *) xmlGetProp (ptr, (xmlChar const *) "scope");
				if (prop) {
					if (strchr (prop, '2') != NULL)
						l.supports2D = true;
					if (strchr (prop, '3') != NULL)
						l.supports3D = true;
					if (strchr (prop, 'c') != NULL || strchr (prop, 'C') != NULL)
						l.supportsCrystals = true;
					if (strchr (prop, 's') != NULL || strchr (prop, 'S') != NULL)
						l.supportsSpectra = true;
					xmlFree (prop);
				}
				loaders[name] = l;
				services[name] = service;
				xmlFree (name);
			}
		}
}

static char *
plugin_service_chemical_loader_get_description (G_GNUC_UNUSED GOPluginService * service)
{
	return g_strdup (_("Chemical file loader type."));
}

static void
plugin_service_chemical_loader_class_init (GObjectClass *gobject_class)
{
	GOPluginServiceClass *plugin_service_class = GO_PLUGIN_SERVICE_CLASS (gobject_class);

	plugin_service_class->read_xml = plugin_service_chemical_loader_read_xml;
	plugin_service_class->get_description = plugin_service_chemical_loader_get_description;
}

static void
plugin_service_chemical_loader_init (G_GNUC_UNUSED PluginServiceChemicalLoader *s)
{
}

GSF_CLASS (PluginServiceChemicalLoader, plugin_service_chemical_loader,
           plugin_service_chemical_loader_class_init, plugin_service_chemical_loader_init,
           GO_TYPE_PLUGIN_SERVICE_SIMPLE)

Loader::Loader ()
{
}

Loader::~Loader ()
{
}

bool Loader::Inited = false;

void Loader::Init (Application *app)
{
	if (Inited)
		return;
	go_plugin_service_define ("chemical_loader",
		&plugin_service_chemical_loader_get_type);
	go_plugin_loader_module_register_version ("gchemutils", VERSION);
	char *plugins_dir = g_strdup (GCU_PLUGINS_DIR);
	GSList *dirs = g_slist_prepend (NULL, plugins_dir);
	go_plugins_init ((app->GetCmdContext ())? app->GetCmdContext ()->GetGOCmdContext (): NULL, NULL, NULL, dirs, true, GO_TYPE_PLUGIN_LOADER_MODULE);
	// do not free dirs and plugins_dir, goffice will
	Inited = true;
}

bool Loader::GetFirstLoader (std::map<std::string, LoaderStruct>::iterator &it)
{
	it = loaders.begin ();
	return it != loaders.end ();
}

bool Loader::GetNextLoader (std::map<std::string, LoaderStruct>::iterator &it)
{
	it++;
	return it != loaders.end ();
}

Loader *Loader::GetLoader (char const *mime_type)
{
	map<string, LoaderStruct>::iterator it = loaders.find (mime_type);
	if (it != loaders.end () && (*it).second.read) {
		if ((*it).second.loader == NULL) {
			GOErrorInfo *error = NULL;
			go_plugin_service_load (services[mime_type], &error);
			if (error) {
				g_warning ("%s", go_error_info_peek_message (error));
				g_free (error);
			}
		}
		return (*it).second.loader;
	} else
		return NULL;
}

Loader *Loader::GetSaver (char const *mime_type)
{
	map<string, LoaderStruct>::iterator it = loaders.find (mime_type);
	if (it != loaders.end () && (*it).second.write) {
		if ((*it).second.loader == NULL) {
			GOErrorInfo *error = NULL;
			go_plugin_service_load (services[mime_type], &error);
			if (error) {
				g_warning ("%s", go_error_info_peek_message (error));
				g_free (error);
			}
		}
		return (*it).second.loader;
	} else
		return NULL;
}

void Loader::AddMimeType (const char *mime_type)
{
	MimeTypes.push_front (mime_type);
	map<string, LoaderStruct>::iterator it = loaders.find (mime_type);
	if (it != loaders.end ())
		(*it).second.loader = this;
}

void Loader::RemoveMimeType (const char *mime_type)
{
	MimeTypes.remove (mime_type);
	map<string, LoaderStruct>::iterator it = loaders.find (mime_type);
	if (it != loaders.end ())
		(*it).second.loader = NULL;
}

ContentType Loader::Read (G_GNUC_UNUSED Document *doc, G_GNUC_UNUSED GsfInput *in, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io)
{
	return ContentTypeUnknown;
}

bool Loader::Write (G_GNUC_UNUSED Object const *obj, G_GNUC_UNUSED GsfOutput *out, G_GNUC_UNUSED char const *mime_type, G_GNUC_UNUSED GOIOContext *io, G_GNUC_UNUSED ContentType type)
{
	return false;
}

}
