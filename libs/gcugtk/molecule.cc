// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/molecule.cc
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "molecule.h"
#include <gcu/application.h>
#include <gcu/document.h>
#include <gsf/gsf-input-stdio.h>
#include <glib/gi18n-lib.h>
#include <vector>

namespace gcugtk {

typedef struct {
	std::string name;
	std::string uri;
	std::string classname;
} BaseAccess;

class MoleculePrivate
{
public:
	static void LoadDatabases (char const *filename);
	static void ShowDatabase (GObject *action);

	static std::vector < BaseAccess > Databases;
};

std::vector < BaseAccess > MoleculePrivate::Databases;

static void
database_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	BaseAccess *state = static_cast < BaseAccess * > (xin->user_state);
	if (state->classname == "molecule" && state->name.length () && state->uri.length ())
		MoleculePrivate::Databases.push_back (*state);
	state->name.clear ();
	state->uri.clear ();
	state->classname.clear ();
};

static void
database_name_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	BaseAccess *state = static_cast < BaseAccess * > (xin->user_state);
	state->name = _(xin->content->str); // this one might be translated
};

static void
database_uri_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	BaseAccess *state = static_cast < BaseAccess * > (xin->user_state);
	state->uri = xin->content->str;
};

static void
database_class_end (GsfXMLIn *xin, G_GNUC_UNUSED GsfXMLBlob *blob)
{
	BaseAccess *state = static_cast < BaseAccess * > (xin->user_state);
	state->classname = xin->content->str;
};

static GsfXMLInNode const databases_dtd[] = {
GSF_XML_IN_NODE (DATABASES, DATABASES, -1, "databases", GSF_XML_NO_CONTENT, NULL, NULL),
	GSF_XML_IN_NODE (DATABASES, DATABASE, -1, "database", GSF_XML_NO_CONTENT, NULL, database_end),
		GSF_XML_IN_NODE (DATABASE, NAME, -1, "name", GSF_XML_CONTENT, NULL, database_name_end),
		GSF_XML_IN_NODE (DATABASE, URI, -1, "uri", GSF_XML_CONTENT, NULL, database_uri_end),
		GSF_XML_IN_NODE (DATABASE, CLASS, -1, "class", GSF_XML_CONTENT, NULL, database_class_end),
	GSF_XML_IN_NODE_END
};

void MoleculePrivate::LoadDatabases (char const *filename)
{
	GError *err = NULL;
	GsfInput *input = gsf_input_stdio_new (filename, &err);
	if (err) {
		g_error_free (err);
		return;
	}
	GsfXMLInDoc *xml = gsf_xml_in_doc_new (databases_dtd, NULL);
	BaseAccess state;
	gsf_xml_in_doc_parse (xml, input, &state);

	gsf_xml_in_doc_free (xml);
	g_object_unref (input);
}

typedef struct {
	Molecule *mol;
	BaseAccess *access;
} AccessState;

void MoleculePrivate::ShowDatabase (GObject *action)
{
	AccessState *state = static_cast < AccessState * > (g_object_get_data (action, "state"));
	if (state) {
		std::string uri = state->access->uri;
		size_t pos = uri.find ('%');
		std::string key;
		switch (uri[pos + 1]) {
		case 'I':
			key = state->mol->GetInChI ();
			break;
		case 'K':
			key = state->mol->GetInChIKey ();
			break;
		case 'S':
			key = state->mol->GetSMILES ();
			break;
		default:
			return;
		}
		if (key.length () == 0)
			return;
		char *escaped = g_uri_escape_string (key.c_str (), NULL, false);
		uri.replace (pos, 2, escaped);
		g_free (escaped);
		static_cast < gcu::Document * > (state->mol->GetDocument ())->GetApplication ()->ShowURI (uri);
	}
}

Molecule::Molecule (gcu::TypeId Type, gcu::ContentType ct): gcu::Molecule (Type, ct)
{
}

Molecule::Molecule (gcu::Atom* pAtom, gcu::ContentType ct): gcu::Molecule (pAtom, ct)
{
}

Molecule::~Molecule ()
{
}

void Molecule::BuildDatabasesMenu (GtkUIManager *UIManager, char const *path_start, char const *path_end)
{
		// add databases submenu
	GtkActionGroup *group = gtk_action_group_new ("databases");
	GtkAction *action;
	if (MoleculePrivate::Databases.empty ()) {
		// load them using gsf xml support
		// first $DATADIR/gchemutils/API_VER/databases.xml
		MoleculePrivate::LoadDatabases (DATADIR"/gchemutils/"GCU_API_VER"/databases.xml");
		// and now $HOME/.gchemutils/datatases.xml
		std::string home = getenv ("HOME");
		home += "/.gchemutils/databases.xml";
		MoleculePrivate::LoadDatabases (home.c_str ());
	}
	if (!MoleculePrivate::Databases.empty ()) {
		action = gtk_action_new ("database", _("Find in databases"), NULL, NULL);
		gtk_action_group_add_action (group, action);
		g_object_unref (action);
		std::vector < BaseAccess >::iterator it, end = MoleculePrivate::Databases.end ();
		for (it = MoleculePrivate::Databases.begin (); it != end; it++) {
			AccessState *state = g_new0 (AccessState, 1);
			state->access = &(*it);
			state->mol = this;
			action = gtk_action_new ((*it).name.c_str (), (*it).name.c_str (), NULL, NULL);
			g_object_set_data_full (G_OBJECT (action), "state", state, g_free);
			g_signal_connect (action, "activate", G_CALLBACK (MoleculePrivate::ShowDatabase), NULL);
			gtk_action_group_add_action (group, action);
			g_object_unref (action);
			std::string node = std::string (path_start) + "<menu action='database'><menuitem action='";
			node += (*it).name;
			node += "'/></menu>";
			node += path_end;
			gtk_ui_manager_add_ui_from_string (UIManager, node.c_str (), -1, NULL);
		}
	}
	gtk_ui_manager_insert_action_group (UIManager, group, 0);
	g_object_unref (group);
}

}
