// -*- C++ -*-

/*
 * GChemPaint cycles plugin
 * plugin.cc
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

#include "config.h"
#include "plugin.h"
#include <gcp/application.h>
#include "cycletool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpCyclesPlugin plugin;

gcpCyclesPlugin::gcpCyclesPlugin (): gcp::Plugin ()
{
}

gcpCyclesPlugin::~gcpCyclesPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Cycle3", gcp_c3_24, NULL},
	{"gcp_Cycle4", gcp_c4_24, NULL},
	{"gcp_Cycle5", gcp_c5_24, NULL},
	{"gcp_Cycle6", gcp_c6_24, NULL},
	{"gcp_Cycle7", gcp_c7_24, NULL},
	{"gcp_Cycle8", gcp_c8_24, NULL},
	{"gcp_CycleN", gcp_cn_24, NULL},
	{NULL, NULL, NULL}
};

static GtkRadioActionEntry entries[] = {
	{	"Cycle3", "gcp_Cycle3", N_("Three atoms cycle"), NULL,
		N_("Add a three membered cycle"),
		0	},
	{	"Cycle4", "gcp_Cycle4", N_("Four atoms cycle"), NULL,
		N_("Add a four membered cycle"),
		0	},
	{	"Cycle5", "gcp_Cycle5", N_("Five atoms cycle"), NULL,
		N_("Add a five membered cycle"),
		0	},
	{	"Cycle6", "gcp_Cycle6", N_("Six atoms cycle"), NULL,
		N_("Add a six membered cycle"),
		0	},
	{	"Cycle7", "gcp_Cycle7", N_("Seven atoms cycle"), NULL,
		N_("Add a seven membered cycle"),
		0	},
	{	"Cycle8", "gcp_Cycle8", N_("Eight atoms cycle"), NULL,
		N_("Add an eight membered cycle"),
		0	},
	{	"CycleN", "gcp_CycleN", N_("Variable sized cycle"), NULL,
		N_("Add a cycle"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='CyclesToolbar'>"
"    <toolitem action='Cycle3'/>"
"    <toolitem action='Cycle4'/>"
"    <toolitem action='Cycle5'/>"
"    <toolitem action='Cycle6'/>"
"    <toolitem action='Cycle7'/>"
"    <toolitem action='Cycle8'/>"
"    <toolitem action='CycleN'/>"
"  </toolbar>"
"</ui>";

void gcpCyclesPlugin::Populate (gcp::Application* App)
{
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("CyclesToolbar", 3);
	new gcpCycleTool (App, 3);
	new gcpCycleTool (App, 4);
	new gcpCycleTool (App, 5);
	new gcpCycleTool (App, 6);
	new gcpCycleTool (App, 7);
	new gcpCycleTool (App, 8);
	new gcpNCycleTool (App, 9);
}
