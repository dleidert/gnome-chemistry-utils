// -*- C++ -*-

/*
 * GChemPaint cycles plugin
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
#include <gccv/arc.h>
#include <gccv/canvas.h>
#include <gccv/polygon.h>
#include <gcp/application.h>
#include "cycletool.h"
#include <glib/gi18n-lib.h>

gcpCyclesPlugin plugin;

gcpCyclesPlugin::gcpCyclesPlugin (): gcp::Plugin ()
{
}

gcpCyclesPlugin::~gcpCyclesPlugin ()
{
}

static gcp::ToolDesc tools[] = {
	{   "Cycle3", N_("Three atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "Cycle4", N_("Four atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "Cycle5", N_("Five atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "Cycle6", N_("Six atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "Cycle7", N_("Seven atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "Cycle8", N_("Eight atoms cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   "CycleN", N_("Variable sized cycle"),
		gcp::RingToolbar, 0, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

static void build_tool (gcp::Application* App, int n)
{
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	double a0, da;
	da = M_PI * 2. / n;
	a0 = M_PI - da / 2.;
	std::list < gccv::Point > Pts;
	gccv::Point p;
	for (int i = 0; i < n; i++) {
		p.x = 12. + 11.* cos (a0);
		p.y = 12. + 11. * sin (a0);
		Pts.push_back (p);
		a0 -= da;
	}
	gccv::Polygon *pl = new gccv::Polygon (canvas, Pts);
	pl->SetLineWidth (2.);
	pl->SetAutoColor (true);
	pl->SetFillColor (0);
	tools[n - 3].widget = canvas->GetWidget ();
	new gcpCycleTool (App, n);
}

void gcpCyclesPlugin::Populate (gcp::Application* App)
{
	for (int i = 3; i < 9; i++)
		build_tool (App, i);
	GtkOverlay *l = GTK_OVERLAY (gtk_overlay_new ());
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Arc *arc = new gccv::Arc (canvas, 12., 12., 10., M_PI * .35, 2 * M_PI);
	arc->SetLineWidth (2.);
	arc->SetAutoColor (true);
	gtk_container_add (GTK_CONTAINER (l), canvas->GetWidget ());
	gtk_widget_set_size_request (canvas->GetWidget (), 24, 24);
	GtkWidget *w = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (w), "<span size=\"smaller\">n</span>");
	g_object_set (w, "halign", GTK_ALIGN_END, "valign", GTK_ALIGN_END, NULL);
	gtk_overlay_add_overlay (l, w);
	tools[6].widget = GTK_WIDGET (l);
	App->AddTools (tools);
	new gcpNCycleTool (App, 9);
}
