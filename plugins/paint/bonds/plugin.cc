// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * plugin.cc
 *
 * Copyright (C) 2004-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "bondtool.h"
#include "chaintool.h"
#include "delocalizedtool.h"
#include "newman.h"
#include "gcp-stock-pixbufs.h"
#include "plugin.h"
#include <gcp/application.h>
#include <gcp/settings.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/line.h>
#include <gccv/poly-line.h>
#include <glib/gi18n-lib.h>

gcpBondsPlugin plugin;

gcpBondsPlugin::gcpBondsPlugin (): gcp::Plugin ()
{
}

gcpBondsPlugin::~gcpBondsPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Bond", NULL, NULL},
	{"gcp_Chain", NULL, NULL},
	{"gcp_UpBond", gcp_upbond_24, NULL},
	{"gcp_DownBond", gcp_downbond_24, NULL},
	{"gcp_iDownBond", gcp_idownbond_24, NULL},
	{"gcp_XBond", gcp_xbond_24, NULL},
	{"gcp_ForeBond", gcp_forebond_24, NULL},
	{"gcp_DelocalizedBond", gcp_delocalizedbond_24, NULL},
	{"gcp_Newman", NULL, NULL},
	{NULL, NULL, NULL}
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
	{	"Newman", "gcp_Newman", N_("Newman projection"), NULL,
		N_("Add a bond in Newman projection"),
		0	}
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
"    <toolitem action='Newman'/>"
"  </toolbar>"
"</ui>";

void gcpBondsPlugin::Populate (gcp::Application* App)
{
	/* Build a canvas for the bond tool */
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Line *line = new gccv::Line (canvas, 3., 21., 21., 3.);
	line->SetLineWidth (2.);
	line->SetAutoColor (true);
	icon_descs[0].canvas = canvas;
	App->AddCanvas ("ui/BondsToolbar/Bond", canvas);
	if (gcp::InvertWedgeHashes)
		entries[3].stock_id = "gcp_iDownBond";
	/* Build a canvas for the chain tool */
	std::list < gccv::Point > Points;
	gccv::Point point;
	point.x = 1.;
	point.y = 23.;
	Points.push_back (point);
	point.x = 6.5;
	point.y = 1.;
	Points.push_back (point);
	point.x = 12.;
	point.y = 23.;
	Points.push_back (point);
	point.x = 17.5;
	point.y = 1.;
	Points.push_back (point);
	point.x = 23.;
	point.y = 23.;
	Points.push_back (point);
	canvas = new gccv::Canvas (NULL);
	gccv::PolyLine *pl = new gccv::PolyLine (canvas, Points);
	pl->SetLineWidth (2.);
	pl->SetAutoColor (true);
	icon_descs[1].canvas = canvas;
	App->AddCanvas ("ui/BondsToolbar/Chain", canvas);
	/* Build a canvas for the Newton projection tool */
	canvas = new gccv::Canvas (NULL);
	gccv::Circle *circle = new gccv::Circle (canvas, 11.5, 11.5, 5.);
	circle->SetAutoColor (true);
	circle->SetFillColor (0);
	line = new gccv::Line (canvas, 11.5, 11.5, 11.5, 0.);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 11.5, 16.5, 11.5, 23.);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 11.5, 11.5, 1.5, 17.3);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 11.5, 11.5, 21.5, 17.3);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 7.2, 9., 1.5, 5.7);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 15.8, 9., 21.5, 5.7);
	line->SetAutoColor (true);
	icon_descs[8].canvas = canvas;
	App->AddCanvas ("ui/BondsToolbar/Newman", canvas);
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("BondsToolbar", 2);
	new gcpBondTool (App);
	new gcpChainTool (App);
	new gcpUpBondTool (App);
	new gcpDownBondTool (App);
	new gcpForeBondTool (App);
	new gcpSquiggleBondTool (App);
	new gcpDelocalizedTool (App);
	new gcpNewmanTool (App);
}
