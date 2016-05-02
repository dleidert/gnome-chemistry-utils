// -*- C++ -*-

/*
 * GChemPaint arrows plugin
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
#include <gccv/arc.h>
#include <gccv/arrow.h>
#include <gccv/bezier-arrow.h>
#include <gcp/application.h>
#include "arrowtool.h"
#include "curvedarrowtool.h"
#include "looptool.h"
#include "retrosynthesis.h"
#include "retrosynthesisarrow.h"
#include "retrosynthesisstep.h"
#include <glib/gi18n-lib.h>

gcpArrowsPlugin plugin;

static Object* CreateRetrosynthesis ()
{
	return new gcpRetrosynthesis ();
}

static Object* CreateRetrosynthesisArrow ()
{
	return new gcpRetrosynthesisArrow (NULL);
}

static Object* CreateRetrosynthesisStep ()
{
	return new gcpRetrosynthesisStep ();
}

gcpArrowsPlugin::gcpArrowsPlugin (): gcp::Plugin ()
{
}

gcpArrowsPlugin::~gcpArrowsPlugin ()
{
}

static gcp::ToolDesc tools[] = {
	{   "SimpleArrow", N_("Add an arrow for an irreversible reaction"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "ReversibleArrow", N_("Add a pair of arrows for a reversible reaction"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "RetrosynthesisArrow", N_("Add an arrow for a retrosynthesis step"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "DoubleHeadedArrow", N_("Add a double headed arrow to represent mesomery"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "CurvedArrow", N_("Add a curved arrow to represent an electron pair move"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "Curved1Arrow", N_("Add a curved arrow to represent an single electron move"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   "TolmanLoop", N_("Create a catalytic cycle from existing molecules"),
		gcp::ArrowToolbar, 0, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpArrowsPlugin::Populate (gcp::Application* App)
{
	RetrosynthesisType = App->AddType ("retrosynthesis", CreateRetrosynthesis);
	App->SetCreationLabel (RetrosynthesisType, _("Create a new retrosynthesis pathway"));
	RetrosynthesisArrowType = App->AddType ("retrosynthesis-arrow", CreateRetrosynthesisArrow);
	RetrosynthesisStepType = App->AddType ("retrosynthesis-step", CreateRetrosynthesisStep);
	GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
	bool FullHeads = go_conf_get_bool (node, "full-arrows-heads");
	go_conf_free_node (node);
	// build canvases for tool buttons
	// Simple arrow
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	gccv::Arrow *arrow = new gccv::Arrow (canvas, 1., 12., 23., 12.);
	arrow->SetA (5.);
	arrow->SetB (6.);
	arrow->SetC (3.);
	arrow->SetAutoColor (true);
	arrow->SetLineWidth (2.);
	tools[0].widget = canvas->GetWidget ();
	// Reversible arrow
	canvas = new gccv::Canvas (NULL);
	arrow = new gccv::Arrow (canvas, 1., 10., 23., 10.);
	arrow->SetA (5.);
	arrow->SetB (6.);
	arrow->SetC (3.);
	arrow->SetAutoColor (true);
	arrow->SetLineWidth (2.);
	arrow->SetEndHead (gccv::ArrowHeadLeft);
	arrow = new gccv::Arrow (canvas, 23., 14., 1., 14.);
	arrow->SetA (5.);
	arrow->SetB (6.);
	arrow->SetC (3.);
	arrow->SetAutoColor (true);
	arrow->SetLineWidth (2.);
	arrow->SetEndHead (gccv::ArrowHeadLeft);
	tools[1].widget = canvas->GetWidget ();
	// Retrosynthesis arrow
	canvas = new gccv::Canvas (NULL);
	gccv::Line *line = new gccv::Line (canvas, 1., 9., 18., 9.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	line = new gccv::Line (canvas, 1., 15., 18., 15.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	line = new gccv::Line (canvas, 14., 5., 23., 12.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	line = new gccv::Line (canvas, 14., 19., 23., 12.);
	line->SetAutoColor (true);
	line->SetLineWidth (2.);
	tools[2].widget = canvas->GetWidget ();
	// Mesomery arrow
	canvas = new gccv::Canvas (NULL);
	arrow = new gccv::Arrow (canvas, 1., 12., 23., 12.);
	arrow->SetA (5.);
	arrow->SetB (6.);
	arrow->SetC (3.);
	arrow->SetAutoColor (true);
	arrow->SetLineWidth (2.);
	arrow->SetStartHead (gccv::ArrowHeadFull);
	tools[3].widget = canvas->GetWidget ();
	// Curved arrow with full head
	canvas = new gccv::Canvas (NULL);
	gccv::BezierArrow *ba = new gccv::BezierArrow (canvas);
	ba->SetControlPoints (2., 23., 2., 1., 20., 1., 20., 23.);
	ba->SetAutoColor (true);
	ba->SetLineWidth (2.);
	tools[4].widget = canvas->GetWidget ();
	// Curved arrow with full head
	canvas = new gccv::Canvas (NULL);
	ba = new gccv::BezierArrow (canvas);
	ba->SetControlPoints (2., 23., 2., 1., 20., 1., 20., 23.);
	ba->SetAutoColor (true);
	ba->SetLineWidth (2.);
	ba->SetHead (gccv::ArrowHeadLeft);
	tools[5].widget = canvas->GetWidget ();
	// Reversible arrow
	canvas = new gccv::Canvas (NULL);
	tools[6].widget = canvas->GetWidget ();
	gccv::Arc *arc = new gccv::Arc (canvas, 12., 12., 9., -5. * M_PI / 12., M_PI / 12.);
	arc->SetLineWidth (2.);
	arc->SetAutoColor (true);
	arc->SetHead (gccv::ArrowHeadFull);	
	arc->SetA (5.);
	arc->SetB (6.);
	arc->SetC (3.);
	arc = new gccv::Arc (canvas, 12., 12., 9., M_PI / 4., 3. * M_PI / 4.);
	arc->SetLineWidth (2.);
	arc->SetAutoColor (true);
	arc->SetHead (gccv::ArrowHeadFull);	
	arc->SetA (5.);
	arc->SetB (6.);
	arc->SetC (3.);
	arc = new gccv::Arc (canvas, 12., 12., 9., 11. * M_PI / 12., 17. * M_PI / 12.);
	arc->SetLineWidth (2.);
	arc->SetAutoColor (true);
	arc->SetHead (gccv::ArrowHeadFull);	
	arc->SetA (5.);
	arc->SetB (6.);
	arc->SetC (3.);
	App->AddTools (tools);
	new gcpArrowTool (App);
	new gcpArrowTool (App, FullHeads? gcp::FullReversibleArrow: gcp::ReversibleArrow);
	new gcpArrowTool (App, gcpDoubleHeadedArrow);
	new gcpArrowTool (App, gcpDoubleQueuedArrow);
	new gcpCurvedArrowTool (App, "CurvedArrow");
	new gcpCurvedArrowTool (App, "Curved1Arrow");
	new gcpLoopTool (App);
	App->AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-step");
	App->AddRule ("retrosynthesis", RuleMustContain, "retrosynthesis-arrow");
	App->AddRule ("retrosynthesis-step", RuleMustContain, "molecule");
	App->AddRule ("molecule", RuleMayBeIn, "retrosynthesis-step");
	App->AddRule ("retrosynthesis-arrow", RuleMustBeIn, "retrosynthesis");
	App->AddRule ("retrosynthesis-step", RuleMustBeIn, "retrosynthesis");
}
