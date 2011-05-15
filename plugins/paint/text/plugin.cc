// -*- C++ -*-

/* 
 * GChemPaint text plugin
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
#include <gcp/text.h>
#include <gcp/fragment.h>
#include "texttool.h"
#include "fragmenttool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpTextPlugin plugin;

gcpTextPlugin::gcpTextPlugin (): gcp::Plugin ()
{
}

gcpTextPlugin::~gcpTextPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Text", gcp_text_24, NULL},
	{"gcp_Fragment", gcp_fragment_24, NULL},
	{NULL, NULL, NULL}
};

static GtkRadioActionEntry entries[] = {
	{	"Text", "gcp_Text", N_("Text"), NULL,
		N_("Add or modify a text"),
		0	},
	{	"Fragment", "gcp_Fragment", N_("Erase"), NULL,
		N_("Add or modify a group of atoms"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'/>"
"	 <placeholder name='Select2'/>"
"	 <placeholder name='Select3'>"
"	   <separator/>"
"	   <toolitem action='Text'/>"
"	 </placeholder>"
"  </toolbar>"
"  <toolbar name='AtomsToolbar'>"
"	 <placeholder name='Atom1'/>"
"	 <placeholder name='Atom2'>"
"	   <toolitem action='Fragment'/>"
"	 </placeholder>"
"	 <placeholder name='Atom3'/>"
"  </toolbar>"
"</ui>";

void gcpTextPlugin::Populate (gcp::Application* App)
{
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	new gcpTextTool (App);
	new gcpFragmentTool (App);
}
