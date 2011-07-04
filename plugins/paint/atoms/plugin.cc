// -*- C++ -*-

/*
 * GChemPaint atoms plugin
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

static gcp::IconDesc icon_descs[] = {
	{"gcp_Element", gcp_element_24},
	{"gcp_ChargePlus", gcp_chargep_24},
	{"gcp_ChargeMinus", gcp_chargem_24},
	{"gcp_ElectronPair", gcp_elecpair_24},
	{"gcp_UnpairedElectron", gcp_unpairedelec_24},
	{"gcp_Orbit", gcp_orbit_24},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"Element", "gcp_Element", N_("Atom"), NULL,
		N_("Add or modify an atom"),
		0	},
	{	"ChargePlus", "gcp_ChargePlus", N_("Positive Charge"), NULL,
		N_("Increment the charge of an atom"),
		0	},
	{	"ChargeMinus", "gcp_ChargeMinus", N_("Negative Charge"), NULL,
		N_("Decrement the charge of an atom"),
		0	},
	{	"ElectronPair", "gcp_ElectronPair", N_("Electron Pair"), NULL,
		N_("Add an electron pair to an atom"),
		0	},
	{	"UnpairedElectron", "gcp_UnpairedElectron", N_("Unpaired Electron"), NULL,
		N_("Add an unpaired electron to an atom"),
		0	},
	{	"Orbital", "gcp_Orbit", N_("Orbital"), NULL,
		N_("Add or modify an atomic orbital"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='AtomsToolbar'>"
"	 <placeholder name='Atom1'>"
"	   <toolitem action='Element'/>"
"	 </placeholder>"
"	 <placeholder name='Atom2'/>"
"	 <placeholder name='Atom3'>"
"	   <toolitem action='ChargePlus'/>"
"	   <toolitem action='ChargeMinus'/>"
"	   <toolitem action='ElectronPair'/>"
"	   <toolitem action='UnpairedElectron'/>"
"	   <toolitem action='Orbital'/>"
"	 </placeholder>"
"  </toolbar>"
"</ui>";

void gcpAtomsPlugin::Populate (gcp::Application* App)
{
	OrbitalType = App->AddType ("orbital", CreateOrbital);
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	App->RegisterToolbar ("AtomsToolbar", 1);
	new gcpElementTool (App);
	new gcpChargeTool (App, "ChargePlus");
	new gcpChargeTool (App, "ChargeMinus");
	new gcpElectronTool (App, "ElectronPair");
	new gcpElectronTool (App, "UnpairedElectron");
	new gcpOrbitalTool (App);
}
