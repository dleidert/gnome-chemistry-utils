// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * plugin.cc 
 *
 * Copyright (C) 2004-2009 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "curvedarrowtool.h"
#include "retrosynthesis.h"
#include "retrosynthesisarrow.h"
#include "retrosynthesisstep.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

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
}

gcpArrowsPlugin::~gcpArrowsPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_SimpleArrow", gcp_arrow1_24, NULL},
	{"gcp_ReversibleArrow", gcp_arrow2_24, NULL},
	{"gcp_RetrosynthesisArrow", gcp_retrosynth_24, NULL},
	{"gcp_MesomeryArrow", gcp_mesomery_24, NULL},
	{"gcp_CurvedArrow", gcp_curved_24, NULL},
	{"gcp_Curved1Arrow", gcp_curved1_24, NULL},
	{NULL, NULL, NULL}
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
	{	"Curved1Arrow", "gcp_Curved1Arrow", N_("Single electron move arrow"), NULL,
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
"    <toolitem action='CurvedArrow'/>"
"    <toolitem action='Curved1Arrow'/>"
"  </toolbar>"
"</ui>";

void gcpArrowsPlugin::Populate (gcp::Application* App)
{
	RetrosynthesisType = App->AddType ("retrosynthesis", CreateRetrosynthesis);
	App->SetCreationLabel (RetrosynthesisType, _("Create a new retrosynthesis pathway"));
	RetrosynthesisArrowType = App->AddType ("retrosynthesis-arrow", CreateRetrosynthesisArrow);
	RetrosynthesisStepType = App->AddType ("retrosynthesis-step", CreateRetrosynthesisStep);
	GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
	bool FullHeads = go_conf_get_bool (node, "full-arrows-heads");
	go_conf_free_node (node);
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("ArrowsToolbar", 4);
	new gcpArrowTool (App);
	new gcpArrowTool (App, FullHeads? gcp::FullReversibleArrow: gcp::ReversibleArrow);
	new gcpArrowTool (App, gcpDoubleHeadedArrow);
	new gcpArrowTool (App, gcpDoubleQueuedArrow);
	new gcpCurvedArrowTool (App, "CurvedArrow");
	new gcpCurvedArrowTool (App, "Curved1Arrow");
	App->AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-step");
	App->AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-arrow");
	App->AddRule ("retrosynthesis-step", RuleMustContain, "molecule");
	App->AddRule ("molecule", RuleMayBeIn, "retrosynthesis-step");
	App->AddRule ("retrosynthesis-arrow", RuleMustBeIn, "retrosynthesis");
	App->AddRule ("retrosynthesis-step", RuleMustBeIn, "retrosynthesis");
}
