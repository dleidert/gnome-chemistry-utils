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
#include <gccv/canvas.h>
#include <gccv/equation.h>
#include <gcp/application.h>
#include <gcp/text.h>
#include <gcp/fragment.h>
#include "texttool.h"
#include "mathtool.h"
#include "fragmenttool.h"
#include "equation.h"
#include "gcp-stock-pixbufs.h"
#include <glib/gi18n-lib.h>

static gcu::Object *CreateEquation ()
{
	return new gcpEquation (0., 0.);
}

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
	{   "Equation", N_("Add or modify an equation"),
		gcp::SelectionToolbar, 2, NULL, NULL},
	{   "Fragment", N_("Add or modify a group of atoms"),
		gcp::AtomToolbar, 1, NULL, NULL},
	{   NULL, NULL, 0, 0, NULL, NULL}
};

void gcpTextPlugin::Populate (gcp::Application* App)
{
	tools[1].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[1].widget), "<span face=\"serif\" size=\"larger\">T</span>");
	EquationType = App->AddType ("equation", CreateEquation);
	App->AddRule ("reaction-prop", gcu::RuleMayContain, "equation");
	gccv::Canvas *canvas = new gccv::Canvas (NULL);
	LsmDomDocument *math = lsm_dom_implementation_create_document (NULL, "math");
	LsmDomNode *math_element = LSM_DOM_NODE (lsm_dom_document_create_element (math, "math"));
	LsmDomNode *style = LSM_DOM_NODE (lsm_dom_document_create_element (math, "mstyle"));
	LsmDomNode *itex_element = LSM_DOM_NODE (lsm_dom_document_create_element (math, "lasem:itex"));
	LsmDomNode *itex_string = LSM_DOM_NODE (lsm_dom_document_create_text_node (math, "\\sqrt\\alpha"));
	lsm_dom_node_append_child (LSM_DOM_NODE (math), math_element);
	lsm_dom_node_append_child (math_element, style);
	lsm_dom_node_append_child (style, itex_element);
	lsm_dom_node_append_child (itex_element, itex_string);
	lsm_dom_element_set_attribute (LSM_DOM_ELEMENT (style), "displaystyle", "true");
	gccv::Equation *eq = new gccv::Equation (canvas, 12., 12.);
	eq->SetMath (math);
	eq->SetAnchor (gccv::AnchorCenter);
	eq->SetAutoFont (true);
	eq->SetAutoTextColor (true);
	g_object_set_data_full (G_OBJECT (canvas->GetWidget ()), "math", math, g_object_unref);
	eq->SetLineColor (0);
	eq->SetFillColor (0);
	tools[2].widget = canvas->GetWidget ();
	tools[3].widget = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (tools[3].widget), "CH<sub><span size=\"smaller\">2</span></sub>");
	g_object_set (G_OBJECT (tools[2].widget), "margin-top", 3, NULL);
	App->AddTools (tools);
	new gcpTextTool (App);
	new gcpMathTool (App);
	new gcpFragmentTool (App);
}
