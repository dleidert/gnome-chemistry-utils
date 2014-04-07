// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * plugin.cc
 *
 * Copyright (C) 2004-2014 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gccv/hash.h>
#include <gccv/line.h>
#include <gccv/poly-line.h>
#include <gccv/squiggle.h>
#include <glib/gi18n-lib.h>

gcpBondsPlugin plugin;

gcpBondsPlugin::gcpBondsPlugin (): gcp::Plugin ()
{
}

gcpBondsPlugin::~gcpBondsPlugin ()
{
}

static gcp::ToolDesc tools[] = {
	{   "Bond", N_("Add a bond or change the multiplicity of an existing one"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "Chain", N_("Add a chain"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "UpBond", N_("Add a wedge bond"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "DownBond", N_("Add a hash bond"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "SquiggleBond", N_("Add a squiggle bond"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "ForeBond", N_("Add a fore bond"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "DelocalizedBond", N_("Add a delocalized bonds system"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   "Newman", N_("Add a bond in Newman projection"),
		gcp::BondToolbar, 0, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpBondsPlugin::Populate (gcp::Application* App)
{
	/* Build a canvas for the bond tool */
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Line *line = new gccv::Line (canvas, 3., 21., 21., 3.);
	line->SetLineWidth (2.);
	line->SetAutoColor (true);
	tools[0].widget = canvas->GetWidget ();
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
	tools[1].widget = canvas->GetWidget ();
	/* Build a canvas for the wedge bond tool */
	canvas = new gccv::Canvas (NULL);
	gccv::Wedge *wedge = new gccv::Wedge (canvas, 2., 22., 19., 5., 8.);
	wedge->SetAutoColor (true);
	tools[2].widget = canvas->GetWidget ();
	/* Build a canvas for the hash bond tool */
	canvas = new gccv::Canvas (NULL);
	gccv::Hash *hash = (gcp::InvertWedgeHashes)?
		new gccv::Hash (canvas, 2., 22., 19., 5., 8.):
		new gccv::Hash (canvas, 19., 5., 2., 22., 8.);
	hash->SetAutoColor (true);
	hash->SetLineWidth (2.);
	hash->SetLineDist (2.);
	tools[3].widget = canvas->GetWidget ();
	/* Build a canvas for the squiggle bond tool */
	canvas = new gccv::Canvas (NULL);
	gccv::Squiggle *squiggle = new gccv::Squiggle (canvas, 2., 22., 22., 2.);
	squiggle->SetLineWidth (2.);
	squiggle->SetAutoColor (true);
	squiggle->SetWidth (6.);
	squiggle->SetStep (3.);
	tools[4].widget = canvas->GetWidget ();
	/* Build a canvas for the fore bond tool */
	canvas = new gccv::Canvas (NULL);
	line = new gccv::Line (canvas, 3., 21., 21., 3.);
	line->SetLineWidth (6.);
	line->SetAutoColor (true);
	tools[5].widget = canvas->GetWidget ();
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
	tools[7].widget = canvas->GetWidget ();
	App->AddTools (tools);
	new gcpBondTool (App);
	new gcpChainTool (App);
	new gcpUpBondTool (App);
	new gcpDownBondTool (App, hash);
	new gcpForeBondTool (App);
	new gcpSquiggleBondTool (App);
	new gcpDelocalizedTool (App);
	new gcpNewmanTool (App);
}
