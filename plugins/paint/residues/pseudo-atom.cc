// -*- C++ -*-

/*
 * GChemPaint residues plugin
 * pseudo-atom.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "pseudo-atom.h"
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-rect-ellipse.h>

TypeId PseudoAtomType = NoType;

gcpPseudoAtom::gcpPseudoAtom (): gcp::Atom ()
{
	gcu::Atom::SetZ (0);
	Lock (true);
}

gcpPseudoAtom::gcpPseudoAtom (double x, double y): gcp::Atom ()
{
	gcu::Atom::SetZ (0);
	SetCoords (x, y);
	Lock (true);
}

gcpPseudoAtom::~gcpPseudoAtom ()
{
}

void gcpPseudoAtom::Update ()
{
}

void gcpPseudoAtom::Add (GtkWidget* w)
{
	gcp::WidgetData* pData = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (G_OBJECT (w), "data"));
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	GnomeCanvasGroup* group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type(), NULL));
	double x, y, r;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	r = pTheme->GetFontSize () / PANGO_SCALE / 2;
	GnomeCanvasItem *item = gnome_canvas_item_new (
						group,
						gnome_canvas_ellipse_ext_get_type (),
						"x1", x - r,
						"y1", y  - r,
						"x2", x + r,
						"y2", y  + r,
						"fill_color", (pData->IsSelected (this))? gcp::SelectColor: "black",
						NULL);
	g_object_set_data (G_OBJECT (group), "ellipse", item);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (gcp::on_event), w);
	g_object_set_data (G_OBJECT (item), "object", this);
	pData->Items[this] = group;
}

void gcpPseudoAtom::Update (GtkWidget* w)
{
	if (!w)
		return;
	gcp::WidgetData* pData = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (G_OBJECT (w), "data"));
	gcp::Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y, r;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	r = pTheme->GetFontSize () / PANGO_SCALE / 2;
	GnomeCanvasGroup *group = pData->Items[this];
		g_object_set (G_OBJECT (g_object_get_data (G_OBJECT (group), "ellipse")),
							"x1", x - r,
							"y1", y  - r,
							"x2", x + r,
							"y2", y  + r,
							NULL);
}

xmlNodePtr gcpPseudoAtom::Save (xmlDocPtr xml)
{
	return NULL;
}

bool gcpPseudoAtom::Load (xmlNodePtr)
{
	return true;
}

bool gcpPseudoAtom::LoadNode (xmlNodePtr)
{
	return true;
}

void gcpPseudoAtom::SetSelected (GtkWidget* w, int state)
{
	gcp::WidgetData* pData = reinterpret_cast<gcp::WidgetData*> (g_object_get_data (G_OBJECT (w), "data"));
	GnomeCanvasGroup* group = pData->Items[this];
	g_object_set (G_OBJECT(g_object_get_data(G_OBJECT(group), "ellipse")),
				"fill_color", ((state == gcp::SelStateSelected)? gcp::SelectColor: "black"), NULL);
}
