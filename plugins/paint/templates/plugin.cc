// -*- C++ -*-

/*
 * GChemPaint templates plugin
 * plugin.cc
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "plugin.h"
#include <gcp/application.h>
#include "templatetool.h"
#include "templatetree.h"
#include "category.h"
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cstring>

gcpTemplatesPlugin plugin;
extern set<xmlDocPtr> docs;

gcpTemplatesPlugin::gcpTemplatesPlugin(): gcp::Plugin()
{
}

gcpTemplatesPlugin::~gcpTemplatesPlugin()
{
}

static GtkRadioActionEntry entries[] = {
	{	"Templates", GTK_STOCK_INDEX, N_("Templates"), NULL,
		N_("Use or manage templates"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'/>"
"	 <placeholder name='Select2'>"
"	   <separator/>"
"	   <toolitem action='Templates'/>"
"	 </placeholder>"
"	 <placeholder name='Select3'/>"
"  </toolbar>"
"</ui>";

void gcpTemplatesPlugin::Populate (gcp::Application* App)
{
	//Load templates, assuming that users templates are not readonly
	xmlDocPtr doc;
	xmlNodePtr node;
	const char *name;
	char *path;
	GDir* dir = g_dir_open (PKGDATADIR"/paint/templates", 0, NULL);
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	if (dir) {
		while ((name = g_dir_read_name (dir))) {
			if (strcmp (name + strlen (name) - 4, ".xml")) continue;
			name = g_strconcat (PKGDATADIR"/paint/templates/", name, NULL);
			doc = xmlParseFile (name);
			docs.insert (doc);
			node = doc->children;
			if (!strcmp ((char*)node->name, "templates")) ParseNodes (node->children, false);
			g_free ((void*) name);
		}
		g_dir_close (dir);
	}
	path = g_strconcat (getenv ("HOME"), "/.gchempaint/templates", NULL);
	dir = g_dir_open (path, 0, NULL);
	if (dir) {
		bool is_user_templates;
		while ((name = g_dir_read_name (dir))) {
			if (strcmp (name + strlen (name) - 4, ".xml"))
				continue;
			is_user_templates = !strcmp (name, "templates.xml");
			name = g_strconcat (path, "/", name, NULL);
			doc = xmlParseFile (name);
			docs.insert (doc);
			if (is_user_templates)
				user_templates = doc;
			node = doc->children;
			if (!strcmp ((char*)node->name, "templates"))
				ParseNodes (node->children, true);
			g_free ((void*) name);
		}
		g_dir_close (dir);
	} else {
		char* gcppath = g_strconcat (getenv ("HOME"), "/.gchempaint", NULL);
		dir = g_dir_open (gcppath, 0, NULL);
		if (dir)
			g_dir_close (dir);
		else
			mkdir (gcppath, 0x1ed);
		g_free (gcppath) ;
		mkdir (path, 0x1ed);
	}
	g_free (path);

	//Poputate the tools box
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, NULL);
	new gcpTemplateTool (App);
	new gcpTemplateTree (App);
}

void gcpTemplatesPlugin::ParseNodes (xmlNodePtr node, bool writeable)
{
	xmlNodePtr next, child, next_child;
	gcpTemplate* t;
	gcpTemplateCategory *c;
	bool lang_matched, cat_matched;
	xmlChar *category, *name;
	string key;
	char *lang = getenv ("LANG"), *node_lang;
	while (node) {
		next = node->next;
		if (!strcmp ((char*) node->name, "template")) {
			category = name = NULL;
			lang_matched = cat_matched = false;
			t = new gcpTemplate ();
			child = node->children;
			while (child) {
				if (!strcmp ((char*)child->name, "text")) {
					child = child->next;
					continue;
				}
				next_child = child->next;
				if (!strcmp ((char*)child->name, "name")) {
					node_lang = (char*) xmlNodeGetLang (child);
					if (node_lang) {
						if (lang) {
							if (!strcmp (lang, node_lang) || (!lang_matched && !strncmp(lang, node_lang, 2)) ){
								if (name) xmlFree (name);
								name = xmlNodeGetContent (child);
								lang_matched = true;
							}
						}
						xmlFree (node_lang);
					} else if (!node_lang && !lang_matched) {
						if (name) xmlFree (name);
						name = xmlNodeGetContent (child);
					}
				} else if (!strcmp ((char*) child->name, "category")) {
					node_lang = (char*) xmlNodeGetLang (child);
					if (node_lang) {
						if (lang) {
							if (!strcmp (lang, node_lang) || (!cat_matched && !strncmp(lang, node_lang, 2))) {
								if (category) xmlFree (category);
								category = xmlNodeGetContent (child);
								cat_matched = true;
							}
						}
						xmlFree (node_lang);
					} else if (!node_lang && !cat_matched) {
						if (category) xmlFree (category);
						category = xmlNodeGetContent (child);
					}
				} else if (t->node == NULL) {
					t->node = child;
				} else {
					delete t;
					t = NULL;
					break;
				}
				child = next_child;
			}
			if (t) {
				if (t->node) {
					t->name = (char*) name;
					t->category = (char*) category;
					t->writeable = writeable;
					key = string((char*) category) + "/" + (char*) name;
					if (Templates[key]) {
						int i = 0;
						char* str = g_strdup_printf ("%d", i);
						while (Templates[key + str]) {
							g_free (str);
							str = g_strdup_printf ("%d", ++i);
						}
						key += str;
						g_free (str);
					}
					Templates[key] = t;
					key = t->name;
					if (TempbyName[key]) {
						int i = 0;
						char* str = g_strdup_printf ("%d", i);
						while (TempbyName[key + str]) {
							g_free (str);
							str = g_strdup_printf ("%d", ++i);
						}
						key += str;
						g_free (str);
					}
					TempbyName[key] = t;
			}	else
				delete t;
			}
			if (name)
				xmlFree (name);
			if (category) {
				c = TemplateCategories[(char*) category];
				if (c == NULL)
					c = new gcpTemplateCategory ((char*) category);
				xmlFree (category);
			} else {
				c = TemplateCategories[_("Miscellaneous")];
				if (c == NULL)
					c = new gcpTemplateCategory (_("Miscellaneous"));
			}
			c->AddTemplate (t);
		}
		node = next;
	}
}

 void gcpTemplatesPlugin::Clear ()
 {
 	set<xmlDocPtr>::iterator i, iend = docs.end ();
	for (i = docs.begin (); i != iend; i++)
		xmlFreeDoc (*i);
	docs.clear ();
	map<string, gcpTemplate*>::iterator j, jend = Templates.end ();
	for (j = Templates.begin (); j != jend; j++)
		delete (*j).second;
	Templates.clear ();
	TempbyName.clear ();
	map<string, gcpTemplateCategory*>::iterator k, kend =  TemplateCategories.end ();
	for (k = TemplateCategories.begin (); k != kend; k++)
		delete (*k).second;
	TemplateCategories.clear ();
}
