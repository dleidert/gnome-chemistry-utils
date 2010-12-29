// -*- C++ -*-

/* 
 * GChemPaint selection plugin
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
#include <gcp/brackets.h>
#include <gcp/molecule.h>
#include "selectiontool.h"
#include "lassotool.h"
#include "erasertool.h"
#include "group.h"
#include "bracketstool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpSelectionPlugin plugin;

static Object* CreateGroup ()
{
	return new gcpGroup ();
}

gcpSelectionPlugin::gcpSelectionPlugin(): gcp::Plugin()
{
}

gcpSelectionPlugin::~gcpSelectionPlugin()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Selection", gcp_selection_24},
	{"gcp_Eraser", gcp_eraser_24},
	{"gcp_Horiz", gcp_horiz_24},
	{"gcp_Lasso", gcp_lasso_24},
	{"gcp_Vert", gcp_vert_24},
	{"gcp_Rotate", gcp_rotate_24},
	{"gcp_Merge", gcp_merge_24},
	{"gcp_Brackets", gcp_brackets_24},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"Select", "gcp_Selection", N_("Select"), NULL,
		N_("Select one or more objects"),
		0	},
	{	"Erase", "gcp_Eraser", N_("Erase"), NULL,
		N_("Eraser"),
		0	},
	{	"Lasso", "gcp_Lasso", N_("Area selection"), NULL,
		N_("Area selection"),
		0	},
	{	"Brackets", "gcp_Brackets", N_("Brackets"), NULL,
		N_("Brackets"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'>"
"      <toolitem action='Select'/>"
"      <toolitem action='Lasso'/>"
"      <toolitem action='Erase'/>"
"      <toolitem action='Brackets'/>"
"	 </placeholder>"
"	 <placeholder name='Select2'/>"
"	 <placeholder name='Select3'/>"
"  </toolbar>"
"</ui>";

void gcpSelectionPlugin::Populate (gcp::Application* App)
{
	GroupType = App->AddType ("group", CreateGroup);
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("SelectToolbar", 0);
	new gcpSelectionTool (App);
	new gcpLassoTool (App);
	new gcpEraserTool (App);
	new gcpBracketsTool (App);
	App->ActivateTool ("Select", true);
	App->AddRule (gcp::BracketsType, gcu::RuleMayContain, GroupType);
}
