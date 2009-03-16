// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-operator.cc 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "document.h"
#include "reaction-operator.h"
#include "settings.h"
#include "theme.h"
#include "view.h"
#include "widgetdata.h"
#include <gccv/canvas.h>
#include <gccv/text.h>
#include <cmath>
#include <cstring>

using namespace gcu;

namespace gcp {

ReactionOperator::ReactionOperator ():
	Object (ReactionOperatorType),
	gccv::ItemClient ()
{
}

ReactionOperator::~ReactionOperator ()
{
}

void ReactionOperator::AddItem ()
{
	Document *doc = static_cast <Document*> (GetDocument ());
	View *view = doc->GetView ();
	Theme *theme = doc ->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= theme->GetZoomFactor ();
	y *= theme->GetZoomFactor ();
	gccv::Text *text = new gccv::Text (view->GetCanvas ()->GetRoot (), x, y, this);
	text->SetFillColor ((view->GetData ()->IsSelected (this))? SelectColor: 0);
	text->SetPadding (theme->GetPadding ());
	text->SetLineColor (0);
	text->SetLineWidth (0.);
	text->SetFontDescription (view->GetPangoFontDesc ());
	text->SetText ("+");
	text->SetLineOffset (view->GetCHeight ());
	m_Item = text;
}

/*void ReactionOperator::Add (GtkWidget* w) const
{
	if (!w)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	if (pData->Items[this] != NULL)
		return;
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	double dFontHeight = pData->m_View->GetFontHeight ();
	GnomeCanvasItem* item;
	GnomeCanvasGroup* group;
	gint width, height;
	PangoContext* pc = gccv::Text::GetContext ();
	group = GNOME_CANVAS_GROUP (gnome_canvas_item_new (pData->Group, gnome_canvas_group_ext_get_type (), NULL));
	pData->Items[this] = group;
	g_signal_connect (G_OBJECT (group), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "object", (void *) this);
	const gchar* symbol = "+";
	const_cast <ReactionOperator *> (this)->m_Layout = pango_layout_new (pc);
	pango_layout_set_text (m_Layout, symbol, strlen (symbol));
	PangoRectangle rect;
	pango_layout_get_extents (m_Layout, &rect, NULL);
	width = rect.width / PANGO_SCALE;
	height =  rect.height / PANGO_SCALE;
	item = gnome_canvas_item_new (
						group,
						gnome_canvas_rect_ext_get_type (),
						"x1", x - (double) width / 2 - pTheme->GetPadding (),
						"y1", y - dFontHeight / 2 - pTheme->GetPadding (),
						"x2", x + (double) width / 2 + pTheme->GetPadding (),
						"y2", y + dFontHeight / 2 + pTheme->GetPadding (),
						"fill_color", "white",
						NULL);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "background",item);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
	item = gnome_canvas_item_new (
						group,
						gnome_canvas_pango_get_type (),
						"layout", m_Layout,
						"x", rint (x),
						"y", rint (y),
						"anchor", GTK_ANCHOR_CENTER,
						"fill_color", (pData->IsSelected (this))? SelectColor: Color,
						NULL);
	g_signal_connect (G_OBJECT (item), "event", G_CALLBACK (on_event), w);
	g_object_set_data (G_OBJECT (group), "text",item);
	g_object_set_data (G_OBJECT (item), "object", (void *) this);
}

void ReactionOperator::Update (GtkWidget* w) const
{
	if (!w)
		return;
	WidgetData* pData = (WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	Theme *pTheme = pData->m_View->GetDoc ()->GetTheme ();
	double x, y;
	GetCoords (&x, &y);
	x *= pTheme->GetZoomFactor ();
	y *= pTheme->GetZoomFactor ();
	double dFontHeight = pData->m_View->GetFontHeight ();
	GnomeCanvasItem* item;
	GnomeCanvasGroup* group = pData->Items[this];
	gint width, height;
	PangoContext* pc = gccv::Text::GetContext ();
	const gchar* symbol = "+";
	PangoLayout *pl = pango_layout_new (pc);
	pango_layout_set_text (pl, symbol, strlen (symbol));
	PangoRectangle rect;
	pango_layout_get_extents (pl, &rect, NULL);
	width = rect.width / PANGO_SCALE;
	height =  rect.height / PANGO_SCALE;
	item = (GnomeCanvasItem*) g_object_get_data (G_OBJECT (group), "background");
	g_object_set (G_OBJECT (item),
						"x1", x - (double) width / 2 - pTheme->GetPadding (),
						"y1", y - dFontHeight / 2 - pTheme->GetPadding (),
						"x2", x + (double) width / 2 + pTheme->GetPadding (),
						"y2", y + dFontHeight / 2 + pTheme->GetPadding (),
						NULL);
	item = (GnomeCanvasItem*) g_object_get_data (G_OBJECT (group), "text");
	g_object_set (G_OBJECT (item),
						"x", rint (x),
						"y", rint (y),
						NULL);
}*/

void ReactionOperator::SetSelected (int state)
{
	GOColor color;
	switch (state) {	
	case SelStateUnselected:
		color = Color;
		break;
	case SelStateSelected:
		color = SelectColor;
		break;
	case SelStateUpdating:
		color = AddColor;
		break;
	case SelStateErasing:
		color = DeleteColor;
		break;
	default:
		color = Color;
		break;
	}
//	dynamic_cast <gccv::Text *> (m_Item)->SetLineColor (color);
}

void ReactionOperator::Move (double x, double y, G_GNUC_UNUSED double z)
{
	m_x += x;
	m_y += y;
}

void ReactionOperator::SetCoords (double x, double y)
{
	m_x = x;
	m_y = y;
}

bool ReactionOperator::GetCoords (double* x, double* y) const
{
	*x = m_x;
	*y = m_y;
	return true;
}
	
double ReactionOperator::GetYAlign ()
{
	return m_y;
}

}	//	namespace gcp
