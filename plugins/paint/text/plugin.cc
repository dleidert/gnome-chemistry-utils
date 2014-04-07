// -*- C++ -*-

/*
 * GChemPaint text plugin
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
	
static gcp::ToolDesc tools[] = {
	{   "Separator", NULL,
		gcp::SelectionToolbar, 2, NULL, NULL},
	{   "Text", N_("Add or modify a text"),
		gcp::SelectionToolbar, 2, NULL, NULL},
	{   "Fragment", N_("Add or modify a group of atoms"),
		gcp::AtomToolbar, 1, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpTextPlugin::Populate (gcp::Application* App)
{
	tools[1].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[1].widget), "<span face=\"serif\" size=\"larger\">T</span>");
	tools[2].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[2].widget), "CH<sub><span size=\"smaller\">2</span></sub>");
	g_object_set (G_OBJECT (tools[2].widget), "margin-top", 3, NULL);
	App->AddTools (tools);
	new gcpTextTool (App);
	new gcpFragmentTool (App);
}
