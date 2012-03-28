// -*- C++ -*-

/*
 * GChemPaint templates plugin
 * templatetree.h
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEMPLATETREE_H
#define GCHEMPAINT_TEMPLATETREE_H

#include <gcp/tool.h>
#include <gccv/structs.h>
#include <map>
#include <string>
#include <set>
#include <libxml/tree.h>
#include <gtk/gtk.h>

using namespace std;

namespace gcp {
	class Document;
	class WidgetData;
}

class gcpTemplate
{
public:
	gcpTemplate ();
	~gcpTemplate();

	string &GetName () {return name;}

	string name;
	string category;
	bool writeable;
	xmlNodePtr node;
	gcp::Document *doc;
	gccv::Rect rect;
	GtkWidget *w;
	double bond_length;
	gcp::WidgetData *data;
};

extern map<string, gcpTemplate*> Templates, TempbyName;
extern set<string> categories;
extern xmlDocPtr user_templates;

//Template tree item
class gcpTemplateTree: public gcp::Tool
{
public:
	gcpTemplateTree (gcp::Application* App);
	~gcpTemplateTree ();

	GtkTreeModel* GetModel () {return (GtkTreeModel*) model;}
	gcpTemplate* GetTemplate (string path);
	void SetTemplate (gcpTemplate *t);
	const char* GetPath (gcpTemplate *t);
	void AddTemplate (string& key);
	void DeleteTemplate (string& key);
	void UpdateMaps ();
	void SetCombo (GtkWidget *w) {combo = GTK_COMBO_BOX (w);}

private:
	GtkTreeStore *model;
	GtkComboBox *combo;
	map<string, gcpTemplate*> m_Templates;
	map<gcpTemplate*, string> m_Paths;
};

#endif //GCHEMPAINT_TEMPLATETREE_H
