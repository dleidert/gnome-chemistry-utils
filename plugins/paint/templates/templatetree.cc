// -*- C++ -*-

/* 
 * GChemPaint templates plugin
 * templatetree.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "templatetree.h"
#include "templatetool.h"
#include "gtkcombotoolitem.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <glib/gi18n-lib.h>

map<string, gcpTemplate*> Templates, TempbyName;
set<xmlDocPtr> docs;
xmlDocPtr user_templates = NULL;

gcpTemplate::gcpTemplate ():
	node (NULL),
	doc (NULL),
	bond_length (0.)
{
}

gcpTemplate::~gcpTemplate ()
{
	if (doc)
		delete (doc);
}

enum {
	NAME_COLUMN,
	NUM_COLUMNS
};

typedef struct
{
	char* name;
} TreeItem;

gcpTemplateTree::gcpTemplateTree (gcp::Application* App): gcp::Tool (App, "TemplateTree")
{
	model = gtk_tree_store_new (NUM_COLUMNS, G_TYPE_STRING);
	GtkTreeIter iter, child_iter;
	map<string, gcpTemplate*>::iterator i, end = Templates.end ();
	string category;
	for (i = Templates.begin(); i!= end; i++) {
		if (category != (*i).second->category) {
			category = (*i).second->category;
			gtk_tree_store_append (model, &iter, NULL);
			gtk_tree_store_set (model, &iter,
				  NAME_COLUMN, category.c_str(),
				  -1);
		}
	  gtk_tree_store_append (model, &child_iter, &iter);
	  gtk_tree_store_set (model, &child_iter,
				  NAME_COLUMN, (*i).second->name.c_str(),
				  -1);
		GtkTreePath *path = gtk_tree_model_get_path ((GtkTreeModel*)model, &child_iter);
		char* s = gtk_tree_path_to_string (path);
		m_Templates[s] = (*i).second;
		m_Paths[(*i).second] = s;
		g_free (s);
		gtk_tree_path_free (path);
	}
}

gcpTemplateTree::~gcpTemplateTree ()
{
}

void gcpTemplateTree::SetTemplate (gcpTemplate *t)
{
	GtkTreePath* path = (t)? gtk_tree_path_new_from_string (m_Paths[t].c_str()): NULL;
	GtkTreeIter iter;
	if (path) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
		gtk_combo_box_set_active_iter (combo, &iter);
		gtk_tree_path_free (path);
	} else
		gtk_combo_box_set_active (combo, 0);
}

gcpTemplate* gcpTemplateTree::GetTemplate (string path)
{
	return m_Templates[path];
}

const char* gcpTemplateTree::GetPath (gcpTemplate *t)
{
	return m_Paths[t].c_str();
}

void gcpTemplateTree::AddTemplate (string& key)
{
	map<string, gcpTemplate*>::iterator i = Templates.find (key), end = Templates.end ();
	GtkTreeIter iter, parent;
	GtkTreePath *path;
	gcpTemplate *t, *new_temp = (*i).second;
	if (i == Templates.begin()) {
		i++;
		if (i == end || ((*i).second->category != new_temp->category)) {
			gtk_tree_store_prepend (model, &parent, NULL);
			gtk_tree_store_set (model, &parent,
				  NAME_COLUMN, new_temp->category.c_str(),
				  -1);
		} else {
			path = gtk_tree_path_new_from_string (GetPath ((*i).second));
			gtk_tree_path_up (path);
			gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &parent, path);
			gtk_tree_path_free (path);
		}
		gtk_tree_store_prepend (model, &iter, &parent);
		gtk_tree_store_set (model, &iter,
				  NAME_COLUMN, new_temp->name.c_str(),
				  -1);
	} else {
		i--;
		t = (*i).second;
		if (t->category == new_temp->category) {
			path = gtk_tree_path_new_from_string (GetPath (t));
			gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
			gtk_tree_path_up (path);
			gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &parent, path);
			gtk_tree_path_free (path);
			gtk_tree_store_insert_after (model, &iter, &parent, &iter);
			gtk_tree_store_set (model, &iter,
					  NAME_COLUMN, new_temp->name.c_str(),
					  -1);
		} else {
			i++;
			i++;
			t = (*i).second;
			if (i != Templates.end() && (t->category == new_temp->category)) {
				path = gtk_tree_path_new_from_string (GetPath (t));
			gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
			gtk_tree_path_up (path);
			gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &parent, path);
			gtk_tree_path_free (path);
			gtk_tree_store_insert_before (model, &iter, &parent, &iter);
			gtk_tree_store_set (model, &iter,
					  NAME_COLUMN, new_temp->name.c_str(),
					  -1);
			} else {
				//new category, must go backwards
				i--;
				i--;
				path = gtk_tree_path_new_from_string (GetPath ((*i).second));
				gtk_tree_path_up (path);
				gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
				gtk_tree_store_insert_after (model, &parent, NULL, &iter);
				gtk_tree_store_set (model, &parent,
					  NAME_COLUMN, new_temp->category.c_str(),
					  -1);
				gtk_tree_store_append (model, &iter, &parent);
				gtk_tree_store_set (model, &iter,
						  NAME_COLUMN, new_temp->name.c_str(),
						  -1);
			}
		}
	}
	UpdateMaps ();
}

void gcpTemplateTree::DeleteTemplate (string& key)
{
	gcpTemplate *t = Templates[key];
	GtkTreeIter iter, parent;
	GtkTreePath *path, *path1;
	const char *s = m_Paths[t].c_str();
	path = gtk_tree_path_new_from_string (s);
	path1 = gtk_tree_path_copy (path);
	gtk_tree_path_up (path1);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &parent, path1);
	gtk_tree_store_remove (model, &iter);
	if (!gtk_tree_model_iter_has_child (GTK_TREE_MODEL (model), &parent)) {
		gtk_tree_store_remove (model, &parent);
		categories.erase (t->category);
	}
	gtk_tree_path_next (path);
	gtk_tree_path_next (path1);
	xmlNodePtr node = t->node->parent;
	xmlDocPtr doc = t->node->doc;
	xmlUnlinkNode (node);
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	xmlSaveFormatFile((char*) doc->URL, doc, true);
	xmlFreeNode (node);
	Templates.erase (key);
	delete t;
	UpdateMaps ();
}

void gcpTemplateTree::UpdateMaps ()
{
	map<string, gcpTemplate*>::iterator i, end = Templates.end ();
	GtkTreePath *path;
	char* s;
	string category;
	path = gtk_tree_path_new_first ();
	gtk_tree_path_down (path);
	//rebuild the maps
	m_Paths.clear ();
	m_Templates.clear ();
	i = Templates.begin ();
	category = (*i).second->category;
	while (i != end) {
		if ((*i).second->category != category) {
			category = (*i).second->category;
			gtk_tree_path_up (path);
			gtk_tree_path_next (path);
			gtk_tree_path_down (path);
		}
		s = gtk_tree_path_to_string (path);
		m_Templates[s] = (*i).second;
		m_Paths[(*i).second] = s;
		g_free (s);
		gtk_tree_path_next (path);
		i++;
	}
	gtk_tree_path_free (path);
}
