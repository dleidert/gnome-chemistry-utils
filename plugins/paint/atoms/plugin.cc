// -*- C++ -*-

/*
 * GChemPaint atoms plugin
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
#include "plugin.h"
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/leaf.h>
#include <gcp/application.h>
#include <gcp/molecule.h>
#include "elementtool.h"
#include "chargetool.h"
#include "electrontool.h"
#include "orbital.h"
#include "orbitaltool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpAtomsPlugin plugin;

static gcu::Object *CreateOrbital ()
{
	return new gcpOrbital (NULL, GCP_ORBITAL_TYPE_S);
}

gcpAtomsPlugin::gcpAtomsPlugin(): gcp::Plugin()
{
}

gcpAtomsPlugin::~gcpAtomsPlugin()
{
}

static gcp::ToolDesc tools[] = {
	{   "Element", N_("Add or modify an atom"),
		gcp::AtomToolbar, 0, NULL, NULL},
	{   "ChargePlus", N_("Increment the charge of an atom"),
		gcp::AtomToolbar, 2, NULL, NULL},
	{   "ChargeMinus", N_("Decrement the charge of an atom"),
		gcp::AtomToolbar, 2, NULL, NULL},
	{   "ElectronPair", N_("Add an electron pair to an atom"),
		gcp::AtomToolbar, 2, NULL, NULL},
	{   "UnpairedElectron", N_("Add an unpaired electron to an atom"),
		gcp::AtomToolbar, 2, NULL, NULL},
	{   "Orbital", N_("Add or modify an atomic orbital"),
		gcp::AtomToolbar, 2, NULL, NULL},

	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpAtomsPlugin::Populate (gcp::Application* App)
{
	OrbitalType = App->AddType ("orbital", CreateOrbital);
	tools[0].widget = gtk_label_new ("C");
	tools[1].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[1].widget), "<span size=\"larger\">⊕</span>");
	tools[2].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[2].widget), "<span size=\"larger\">⊖</span>");
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Circle *circle = new gccv::Circle (canvas, 12., 9., 1.);
	circle->SetLineWidth (2.);
	circle->SetAutoColor (true);
	circle = new gccv::Circle (canvas, 12., 15., 1.);
	circle->SetLineWidth (2.);
	circle->SetAutoColor (true);
	tools[3].widget = canvas->GetWidget ();
	canvas = new gccv::Canvas (NULL);
	circle = new gccv::Circle (canvas, 12., 12., 1.);
	circle->SetLineWidth (2.);
	circle->SetAutoColor (true);
	tools[4].widget = canvas->GetWidget ();
	canvas = new gccv::Canvas (NULL);
	gccv::Leaf *leaf = new gccv::Leaf (canvas, 12., 12., 11.);
	leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
	leaf->SetLineWidth (1.);
	leaf->SetAutoColor (true);
	leaf->SetFillColor (GO_COLOR_GREY (100));
	leaf = new gccv::Leaf (canvas, 12., 12., 11.);
	leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
	leaf->SetRotation (M_PI);
	leaf->SetLineWidth (1.);
	leaf->SetAutoColor (true);
	leaf->SetFillColor (GO_COLOR_WHITE);
	tools[5].widget = canvas->GetWidget ();
	App->AddTools (tools);
	new gcpElementTool (App);
	new gcpChargeTool (App, "ChargePlus");
	new gcpChargeTool (App, "ChargeMinus");
	new gcpElectronTool (App, "ElectronPair");
	new gcpElectronTool (App, "UnpairedElectron");
	new gcpOrbitalTool (App);
}
