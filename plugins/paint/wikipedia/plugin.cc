// -*- C++ -*-

/* 
 * GChemPaint Wikipedia plugin
 * plugin.h 
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
#include "wikipediatool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpWikipediaPlugin plugin;

gcpWikipediaPlugin::gcpWikipediaPlugin (): gcp::Plugin ()
{
}

gcpWikipediaPlugin::~gcpWikipediaPlugin ()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Wikipedia", favicon},
	{NULL, NULL},
};

static GtkRadioActionEntry entries[] = {
	{	"Wikipedia", "gcp_Wikipedia", N_("Wikipedia export"), NULL,
		N_("Export for Wikipedia publication"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'/>"
"	 <placeholder name='Select2'/>"
"	 <placeholder name='Select3'>"
"	   <separator/>"
"	   <toolitem action='Wikipedia'/>"
"	 </placeholder>"
"  </toolbar>"
"</ui>";

void gcpWikipediaPlugin::Populate (gcp::Application* App)
{
	App->AddActions (entries, G_N_ELEMENTS (entries), ui_description, icon_descs);
	new gcpWikipediaTool (App);
}
