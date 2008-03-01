// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * plugin.cc 
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
#include "plugin.h"
#include <gcp/application.h>
#include "arrowtool.h"
#include "retrosynthesis.h"
#include "retrosynthesisarrow.h"
#include "retrosynthesisstep.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>
#ifdef HAVE_GO_CONF_SYNC
#	include <goffice/app/go-conf.h>
#else
#	include <gconf/gconf-client.h>
#endif

gcpArrowsPlugin plugin;

static Object* CreateRetrosynthesis ()
{
	return new gcpRetrosynthesis ();
}

static Object* CreateRetrosynthesisArrow ()
{
	return new gcpRetrosynthesisArrow (NULL);
}

static Object* CreateRetrosynthesisStep ()
{
	return new gcpRetrosynthesisStep ();
}

gcpArrowsPlugin::gcpArrowsPlugin (): gcp::Plugin ()
{
	RetrosynthesisType = Object::AddType ("retrosynthesis", CreateRetrosynthesis);
	Object::SetCreationLabel (RetrosynthesisType, _("Create a new retrosynthesis pathway"));
	RetrosynthesisArrowType = Object::AddType ("retrosynthesis-arrow", CreateRetrosynthesisArrow);
	RetrosynthesisStepType = Object::AddType ("retrosynthesis-step", CreateRetrosynthesisStep);
}

gcpArrowsPlugin::~gcpArrowsPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_SimpleArrow", gcp_arrow1_24},
	{"gcp_ReversibleArrow", gcp_arrow2_24},
	{"gcp_RetrosynthesisArrow", gcp_retrosynth_24},
	{"gcp_MesomeryArrow", gcp_mesomery_24},
	{"gcp_CurvedArrow", gcp_curved_24},
	{"gcp_Curved1Arrow", gcp_curved1_24},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"SimpleArrow", "gcp_SimpleArrow", N_("Simple arrow"), NULL,
		N_("Add an arrow for an irreversible reaction"),
		0	},
	{	"ReversibleArrow", "gcp_ReversibleArrow", N_("Double arrow"), NULL,
		N_("Add a pair of arrows for a reversible reaction"),
		0	},
	{	"RetrosynthesisArrow", "gcp_RetrosynthesisArrow", N_("Retrosynthesis arrow"), NULL,
		N_("Add an arrow for a retrosynthesis step"),
		0	},
	{	"DoubleHeadedArrow", "gcp_MesomeryArrow", N_("Mesomery arrow"), NULL,
		N_("Add a double headed arrow to represent mesomery"),
		0	},
	{	"CurvedArrow", "gcp_CurvedArrow", N_("Electron pair move arrow"), NULL,
		N_("Add a curved arrow to represent an electron pair move"),
		0	},
	{	"Curved1Arrow", "gcp_Curved1Arrowd", N_("Single electron move arrow"), NULL,
		N_("Add a curved arrow to represent an single electron move"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='ArrowsToolbar'>"
"    <toolitem action='SimpleArrow'/>"
"    <toolitem action='ReversibleArrow'/>"
"    <toolitem action='RetrosynthesisArrow'/>"
"    <toolitem action='DoubleHeadedArrow'/>"
//"    <toolitem action='CurvedArrow'/>"
//"    <toolitem action='Curved1Arrow'/>"
"  </toolbar>"
"</ui>";

void gcpArrowsPlugin::Populate (gcp::Application* App)
{
#ifdef HAVE_GO_CONF_SYNC
	GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
	bool FullHeads = go_conf_get_bool (node, "full-arrows-heads");
	go_conf_free_node (node);
#else
	GError *error = NULL;
	GConfClient *conf_client = gconf_client_get_default ();
	gconf_client_add_dir (conf_client, "/apps/gchempaint/plugins/arrows", GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	bool FullHeads = gconf_client_get_bool (conf_client, "/apps/gchempaint/plugins/arrows/full-arrows-heads", &error);
	if (error) {
		FullHeads = false;
		g_message("GConf failed: %s", error->message);
		g_error_free (error);
	}
	gconf_client_remove_dir (conf_client, "/apps/gchempaint/plugins/arrows", NULL);
	g_object_unref (conf_client);
#endif
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("ArrowsToolbar", 4);
	new gcpArrowTool (App);
	new gcpArrowTool (App, FullHeads? gcp::FullReversibleArrow: gcp::ReversibleArrow);
	new gcpArrowTool (App, gcpDoubleHeadedArrow);
	new gcpArrowTool (App, gcpDoubleQueuedArrow);
	Object::AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-step");
	Object::AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-arrow");
	Object::AddRule ("retrosynthesis-step", RuleMustContain, "molecule");
	Object::AddRule ("molecule", RuleMayBeIn, "retrosynthesis-step");
	Object::AddRule ("retrosynthesis-arrow", RuleMustBeIn, "retrosynthesis");
	Object::AddRule ("retrosynthesis-step", RuleMustBeIn, "retrosynthesis");
}
