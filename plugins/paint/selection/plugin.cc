// -*- C++ -*-

/*
 * GChemPaint selection plugin
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
#include <gcp/brackets.h>
#include <gcp/molecule.h>
#include <gccv/canvas.h>
#include <gccv/line.h>
#include <gccv/rectangle.h>
#include <gccv/arrow.h>
#include <gccv/path.h>
#include <gccv/polygon.h>
#include "selectiontool.h"
#include "lassotool.h"
#include "erasertool.h"
#include "group.h"
#include "bracketstool.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

gcpSelectionPlugin plugin;

static Object* CreateGroup ()
{
	return new gcpGroup ();
}

gcpSelectionPlugin::gcpSelectionPlugin(): gcp::Plugin()
{
}

gcpSelectionPlugin::~gcpSelectionPlugin()
{
}

static gcp::IconDesc icon_descs[] = {
	{"gcp_Selection", gcp_selection_24, NULL},
	{"gcp_Eraser", gcp_eraser_24, NULL},
	{"gcp_Horiz", gcp_horiz_24, NULL},
	{"gcp_Lasso", gcp_lasso_24, NULL},
	{"gcp_Vert", gcp_vert_24, NULL},
	{"gcp_Rotate", gcp_rotate_24, NULL},
	{"gcp_Merge", NULL, NULL},
	{"gcp_Brackets", gcp_brackets_24, NULL},
	{NULL, NULL, NULL}
};

static GtkRadioActionEntry entries[] = {
	{	"Select", "gcp_Selection", N_("Select"), NULL,
		N_("Select one or more objects"),
		0	},
	{	"Erase", "gcp_Eraser", N_("Erase"), NULL,
		N_("Eraser"),
		0	},
	{	"Lasso", "gcp_Lasso", N_("Area selection"), NULL,
		N_("Area selection"),
		0	},
	{	"Brackets", "gcp_Brackets", N_("Brackets"), NULL,
		N_("Brackets"),
		0	},
};

static const char *ui_description =
"<ui>"
"  <toolbar name='SelectToolbar'>"
"	 <placeholder name='Select1'>"
"      <toolitem action='Select'/>"
"      <toolitem action='Lasso'/>"
"      <toolitem action='Erase'/>"
"      <toolitem action='Brackets'/>"
"	 </placeholder>"
"	 <placeholder name='Select2'/>"
"	 <placeholder name='Select3'/>"
"  </toolbar>"
"</ui>";

static gcp::ToolDesc tools[] = {
	{   "Select", N_("Select one or more objects"),
		gcp::SelectionToolbar, 0, NULL, NULL},
	{   "Lasso", N_("Area selection"),
		gcp::SelectionToolbar, 0, NULL, NULL},
	{   "Erase", N_("Eraser"),
		gcp::SelectionToolbar, 0, NULL, NULL},
	{   "Brackets", N_("Brackets"),
		gcp::SelectionToolbar, 0, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpSelectionPlugin::Populate (gcp::Application* App)
{
	GroupType = App->AddType ("group", CreateGroup);
	// build canvases for tool buttons
	// Select
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Arrow *arrow = new gccv::Arrow (canvas, 16., 20., 8., 4.);
	arrow->SetA (10.);
	arrow->SetB (12.);
	arrow->SetAutoColor (true);
	arrow->SetLineWidth (3.);
	tools[0].widget = canvas->GetWidget ();
	// Lasso
	char const *lasso_path = "m 4.9094888,17.588181 c 1.6201935,0.2826 0.8346452,0.8476 1.9147745,0.1615 1.0801285,-0.6861 1.2478843,-0.6474 1.1292256,-1.5739 -0.098194,-0.7669 -0.4266144,-1.3472 -1.0801292,-1.6144 -0.6503682,-0.2658 -1.5220001,-0.3631 -2.1111607,0.081 -0.6653795,0.5014 -1.0310322,0.6055 -0.8837423,1.4932 0.1472904,0.888 -0.049097,0.5651 0.4909678,1.1301 0.7305159,0.7642 2.7985161,0.9686 4.3205156,1.0089 1.5219999,0.041 3.7313549,-0.3632 5.4988389,-1.0089 1.767483,-0.6458 4.860581,-2.6636 5.842515,-3.955 0.981936,-1.2914 1.472902,-2.6636 1.472902,-4.2779996 0,-1.6141 -0.230043,-3.5572 -2.012966,-4.762 -1.801021,-1.217 -3.978673,-1.4197 -5.940709,-1.4125 -2.163815,0.01 -3.6277786,0.4235 -5.1060642,1.1301 -2.5733374,1.2297 -3.9277425,2.3809 -4.6641936,3.6724 -0.7364505,1.2914 -1.2295904,2.6634996 -1.2765161,3.9145996 -0.034186,0.9115 7e-7,2.8655 0.9819358,3.7936 1.6631208,1.5722 3.2403862,0.1211 3.1912899,2.946 -0.05472,3.1486 -0.4418708,3.0672 -1.0310321,3.5515";
	canvas = new gccv::Canvas (NULL);
	gccv::Path *path = new gccv::Path (canvas, go_path_new_from_svg (lasso_path));
	path->SetAutoColor (true);
	path->SetLineWidth (2.);
	path->SetFillColor (0);
	tools[1].widget = canvas->GetWidget ();
	// Eraser
	canvas = new gccv::Canvas (NULL);
	std::list < gccv::Point > Points;
	gccv::Point point;
	point.x = 3.5;
	point.y = 17.5;
	Points.push_back (point);
	point.x = 3.5;
	point.y = 13.5;
	Points.push_back (point);
	point.x = 11.5;
	point.y = 5.5;
	Points.push_back (point);
	point.x = 19.5;
	point.y = 5.5;
	Points.push_back (point);
	point.x = 19.5;
	point.y = 9.5;
	Points.push_back (point);
	point.x = 11.5;
	point.y = 17.5;
	Points.push_back (point);
	gccv::Line *line = new gccv::Line (canvas, 3.5, 13.5, 11.5, 13.5);
	line->SetLineWidth (1.);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 11.5, 13.5, 19.5, 5.5);
	line->SetLineWidth (1.);
	line->SetAutoColor (true);
	line = new gccv::Line (canvas, 11.5, 13.5, 11.5, 17.5);
	line->SetLineWidth (1.);
	line->SetAutoColor (true);
	gccv::Polygon *pl = new gccv::Polygon (canvas, Points);
	pl->SetLineWidth (1.);
	pl->SetAutoColor (true);
	pl->SetAutoFillColor (true);
	tools[2].widget = canvas->GetWidget ();
	// Brackets
	tools[3].widget = gtk_label_new ("[ ]");
	App->AddTools (tools);
	new gcpSelectionTool (App);
	new gcpLassoTool (App);
	new gcpEraserTool (App);
	new gcpBracketsTool (App);
	App->ActivateTool ("Select", true);
	App->AddRule (gcp::BracketsType, gcu::RuleMayContain, GroupType);
}
