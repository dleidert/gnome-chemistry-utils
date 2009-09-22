// -*- C++ -*-

/* 
 * GChemPaint bonds plugin
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
#include "bondtool.h"
#include "chaintool.h"
#include "delocalizedtool.h"
#include "gcp-stock-pixbufs.h"
#include "plugin.h"
#include <gcp/application.h>
#include <gcp/settings.h>
#include <glib/gi18n-lib.h>

gcpBondsPlugin plugin;

gcpBondsPlugin::gcpBondsPlugin (): gcp::Plugin ()
{
}

gcpBondsPlugin::~gcpBondsPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Bond", gcp_bond_24},
	{"gcp_Chain", gcp_chain_24},
	{"gcp_UpBond", gcp_upbond_24},
	{"gcp_DownBond", gcp_downbond_24},
	{"gcp_iDownBond", gcp_idownbond_24},
	{"gcp_XBond", gcp_xbond_24},
	{"gcp_ForeBond", gcp_forebond_24},
	{"gcp_DelocalizedBond", gcp_delocalizedbond_24},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"Bond", "gcp_Bond", N_("Bond"), NULL,
		N_("Add a bond or change the multiplicity of an existing one"),
		0	},
	{	"Chain", "gcp_Chain", N_("Chain"), NULL,
		N_("Add a chain"),
		0	},
	{	"UpBond", "gcp_UpBond", N_("Wedge bond tool"), NULL,
		N_("Add a wedge bond"),
		0	},
	{	"DownBond", "gcp_DownBond", N_("Hash bond tool"), NULL,
		N_("Add a hash bond"),
		0	},
	{	"SquiggleBond", "gcp_XBond", N_("Squiggle bond tool"), NULL,
		N_("Add a squiggle bond"),
		0	},
	{	"ForeBond", "gcp_ForeBond", N_("Fore bond tool"), NULL,
		N_("Add a fore bond"),
		0	},
	{	"DelocalizedBond", "gcp_DelocalizedBond", N_("Delocalized bond tool"), NULL,
		N_("Add a delocalized bonds system"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='BondsToolbar'>"
"    <toolitem action='Bond'/>"
"    <toolitem action='Chain'/>"
"    <toolitem action='UpBond'/>"
"    <toolitem action='DownBond'/>"
"    <toolitem action='SquiggleBond'/>"
"    <toolitem action='ForeBond'/>"
//"    <toolitem action='DelocalizedBond'/>"
"  </toolbar>"
"</ui>";

void gcpBondsPlugin::Populate (gcp::Application* App)
{
	if (gcp::InvertWedgeHashes)
		entries[3].stock_id = "gcp_iDownBond";
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("BondsToolbar", 2);
	new gcpBondTool (App);
	new gcpChainTool (App);
	new gcpUpBondTool (App);
	new gcpDownBondTool (App);
	new gcpForeBondTool (App);
	new gcpSquiggleBondTool (App);
	new gcpDelocalizedTool (App);
}
