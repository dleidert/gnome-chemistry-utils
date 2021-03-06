// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * plugin.cc
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
#include "plugin.h"
#include "residues-dlg.h"
#include "pseudo-atom.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/molecule.h>
#include <gcp/residue.h>
#include <gcugtk/ui-manager.h>
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cstring>

gcpResiduesPlugin plugin;
extern xmlDocPtr user_residues;
extern set<xmlDocPtr> docs;

static Object* CreatePseudoAtom ()
{
	return new gcpPseudoAtom ();
}

gcpResiduesPlugin::gcpResiduesPlugin (): gcp::Plugin ()
{
}

gcpResiduesPlugin::~gcpResiduesPlugin ()
{
}

static void on_edit_residues ()
{
	plugin.OpenDialog ();
}

static void on_new_residue (gcp::Residue *res)
{
	plugin.OnNewResidue (res);
}

static GOptionEntry options[] =
{
  {"edit-residue", 'e', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void*) on_edit_residues, "Open residues dialog", NULL},
  {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

static GtkActionEntry entries[] = {
	{ "Residues", NULL, N_("_Edit residues..."), NULL,
	  N_("Create new abbreviations"), G_CALLBACK (on_edit_residues) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ToolsMenu'>"
"	   <placeholder name='tools1'>"
"       <menuitem action='Residues'/>"
"	   </placeholder>"
"    </menu>"
"  </menubar>"
"</ui>";

static void on_menu (gcu::UIManager *UIManager)
{
	GtkUIManager *uim = static_cast < gcugtk::UIManager * > (UIManager)->GetUIManager ();
	GtkActionGroup *action_group = gtk_action_group_new ("ResiduesActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), NULL);

	gtk_ui_manager_insert_action_group (uim, action_group, 1);
	g_object_unref (action_group);
	gtk_ui_manager_add_ui_from_string (uim, ui_description, -1, NULL);
}

void gcpResiduesPlugin::Populate (gcp::Application *App)
{
	m_App = App;
	PseudoAtomType = App->AddType ("pseudo-atom", CreatePseudoAtom);
	xmlDocPtr doc;
	xmlNodePtr node;
	char *name;
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	doc = xmlParseFile (PKGDATADIR"/residues.xml");
	if (doc) {
		docs.insert (doc);
		node = doc->children;
		if (!strcmp ((char*)node->name, "residues"))
			ParseNodes (node->children, false);
	}
	char* gcupath = g_strconcat (getenv ("HOME"), "/.gchemutils", NULL);
	GDir* dir = g_dir_open (gcupath, 0, NULL);
	if (dir)
		g_dir_close (dir);
	else
		mkdir (gcupath, 0x1ed);
	g_free (gcupath) ;
	name = g_strconcat (getenv ("HOME"), "/.gchemutils/residues.xml", NULL);
	if (g_file_test (name, G_FILE_TEST_EXISTS) && (doc = xmlParseFile (name))) {
		docs.insert (doc);
		user_residues = doc;
		node = doc->children;
		if (!strcmp ((char*)node->name, "residues"))
			ParseNodes (node->children, true);
	}
	g_free (name);
	App->RegisterOptions (options);
	App->AddMenuCallback (on_menu);
	gcp::Residue::SetPostAddCallback (on_new_residue);
}

void gcpResiduesPlugin::OpenDialog ()
{
	Dialog *dlg = m_App->GetDialog ("residues");
	if (dlg)
		dlg->Present ();
	else
		new gcpResiduesDlg (m_App);
}

void gcpResiduesPlugin::ParseNodes (xmlNodePtr node, bool writeable)
{
	gcp::Residue* r;
	while (node) {
		if (!strcmp ((char*) node->name, "residue")) {
			r = new gcp::Residue ();
			r->Load (node, !writeable, m_App);
		}
		node = node->next;
	}
}

void gcpResiduesPlugin::OnNewResidue (gcp::Residue *res)
{
	Dialog *dlg = m_App->GetDialog ("residues");
	// build the xml node if none exists and save it
	if (res) {
		xmlNodePtr node = res->GetMolNode ();
		if (!node) {
			if (!user_residues) {
				user_residues = xmlNewDoc ((xmlChar*) "1.0");
				docs.insert (user_residues);
				xmlDocSetRootElement (user_residues,  xmlNewDocNode (user_residues, NULL, (xmlChar*) "residues", NULL));
				char* filename = g_strconcat (getenv ("HOME"), "/.gchemutils/residues.xml", NULL);
				user_residues->URL = xmlStrdup ((xmlChar*) filename);
				g_free (filename);
			}
			node = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "residue", NULL);
			if (res->GetGeneric ())
				xmlNewProp (node, (xmlChar const *) "generic", (xmlChar const *) "true");
			string raw = static_cast <gcp::Molecule *> (res->GetMolecule ())->GetRawFormula ();
			xmlNewProp (node, (xmlChar const *) "raw", (xmlChar const *) raw.c_str ());
			map<string, bool> const &symbols = res->GetSymbols ();
			map<string, bool>::const_iterator i = symbols.begin (), end = symbols.end ();
			string sy;
			if (i != symbols.end ())
				sy = (*i).first;
			for (i++; i != end; i++)
				sy += string(";") + (*i).first;
			xmlNodePtr child = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "symbols", (xmlChar const *) sy.c_str ());
			xmlAddChild (node, child);
			child = xmlNewDocNode (user_residues, NULL, (xmlChar const *) "name", (xmlChar const *) res->GetName ());
			xmlAddChild (node, child);
			xmlDocPtr xml = static_cast <gcp::Document *> (res->GetDocument ())->BuildXMLTree ();
			child = xml->children->children;
			while (strcmp ((char const *) child->name, "molecule"))
				child = child->next;
			xmlUnlinkNode (child);
			xmlAddChild (node, child);
			xmlAddChild (user_residues->children, node);
			xmlIndentTreeOutput = true;
			xmlKeepBlanksDefault (0);
			xmlSaveFormatFile ((char*) user_residues->URL, user_residues, true);
			xmlFreeDoc (xml);
			res->Load (node, false, m_App);
		}
	}
	if (dlg)
		static_cast <gcpResiduesDlg *> (dlg)->OnNewResidue (res);
}

void gcpResiduesPlugin::Clear ()
{
	set<xmlDocPtr>::iterator i, iend = docs.end ();
	for (i = docs.begin (); i != iend; i++)
		xmlFreeDoc (*i);
	docs.clear ();
}
