// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * arrowtool.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "arrowtool.h"
#include "retrosynthesisarrow.h"
#include <gcp/settings.h>
#include <canvas/gcp-canvas-line.h>
#include <canvas/gcp-canvas-group.h>
#include <canvas/gcp-canvas-bpath.h>
#include <gcp/document.h>
#include <gcp/application.h>
#include <gcp/mesomery-arrow.h>
#include <gcp/reaction-arrow.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <glib/gi18n-lib.h>
#include <gconf/gconf-client.h>
#include <math.h>

static char* ToolNames[] = {
	"SimpleArrow",
	"ReversibleArrow",
	"ReversibleArrow",
	"DoubleHeadedArrow",
	"RetrosynthesisArrow"
};

static void on_default (GtkToggleButton *button)
{
	GConfClient *conf_client = gconf_client_get_default ();
	gconf_client_set_bool (conf_client, "/apps/gchempaint/plugins/arrows/full-arrows-heads", gtk_toggle_button_get_active (button), NULL);
	g_object_unref (conf_client);
}

gcpArrowTool::gcpArrowTool (gcp::Application* App, unsigned ArrowType): gcp::Tool (App, ToolNames[ArrowType])
{
	points = gnome_canvas_points_new (2);
	m_ArrowType = ArrowType;
}

gcpArrowTool::~gcpArrowTool ()
{
	gnome_canvas_points_free (points);
}	

bool gcpArrowTool::OnClicked ()
{
	if (m_pObject)
		return false;
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	m_y1 = m_y0;
	m_x1 = m_x0 + pDoc->GetArrowLength () * pTheme->GetZoomFactor ();
	switch (m_ArrowType) {
	case gcp::SimpleArrow:
		points->coords[0] = m_x0;
		points->coords[1] = m_y0;
		points->coords[2] = m_x1;
		points->coords[3] = m_y0;
		m_pItem = gnome_canvas_item_new (
									m_pGroup,
									gnome_canvas_line_ext_get_type (),
									"points", points,
									"fill_color", gcp::AddColor,
									"width_units", pTheme->GetArrowWidth (),
									"last_arrowhead", true,
									"arrow_shape_a", pTheme->GetArrowHeadA (),
									"arrow_shape_b", pTheme->GetArrowHeadB (),
									"arrow_shape_c", pTheme->GetArrowHeadC (),
									"last_arrowhead_style", (unsigned char)ARROW_HEAD_BOTH,
									NULL);
		break;
	case gcp::ReversibleArrow:
		points->coords[0] = m_x0;
		points->coords[1] = points->coords[3] = m_y0 - pTheme->GetArrowDist () / 2;
		points->coords[2] = m_x1;
		m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_LEFT,
							NULL);
		points->coords[2] = m_x0;
		points->coords[1] = points->coords[3] = m_y0 + pTheme->GetArrowDist () / 2;
		points->coords[0] = m_x0 + pDoc->GetArrowLength ();
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_LEFT,
							NULL);
		break;
	case gcp::FullReversibleArrow:
		points->coords[0] = m_x0;
		points->coords[1] = points->coords[3] = m_y0 - pTheme->GetArrowDist () / 2;
		points->coords[2] = m_x1;
		m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
							NULL);
		points->coords[2] = m_x0;
		points->coords[1] = points->coords[3] = m_y0 + pTheme->GetArrowDist () / 2;
		points->coords[0] = m_x0 + pDoc->GetArrowLength ();
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
							NULL);
		break;
	case gcpDoubleHeadedArrow:
		points->coords[0] = m_x0;
		points->coords[1] = m_y0;
		points->coords[2] = m_x1;
		points->coords[3] = m_y0;
		m_pItem = gnome_canvas_item_new (
									m_pGroup,
									gnome_canvas_line_ext_get_type (),
									"points", points,
									"fill_color", gcp::AddColor,
									"width_units", pTheme->GetArrowWidth (),
									"first_arrowhead", true,
									"last_arrowhead", true,
									"arrow_shape_a", pTheme->GetArrowHeadA (),
									"arrow_shape_b", pTheme->GetArrowHeadB (),
									"arrow_shape_c", pTheme->GetArrowHeadC (),
									"first_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
									"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
									NULL);
		break;
	case gcpDoubleQueuedArrow: {
		GnomeCanvasPathDef* path = gnome_canvas_path_def_new ();
		gnome_canvas_path_def_moveto (path, m_x0, m_y0 - pTheme->GetArrowDist () / 2.);
		gnome_canvas_path_def_lineto (path, m_x1 - pTheme->GetArrowDist () / 2., m_y0 - pTheme->GetArrowDist () / 2.);
		gnome_canvas_path_def_moveto (path, m_x0, m_y0 + pTheme->GetArrowDist () / 2.);
		gnome_canvas_path_def_lineto (path, m_x1 - pTheme->GetArrowDist () / 2., m_y0 + pTheme->GetArrowDist () / 2.);
		gnome_canvas_path_def_moveto (path, m_x1 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC (), m_y0 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC ());
		gnome_canvas_path_def_lineto (path, m_x1, m_y0);
		gnome_canvas_path_def_lineto (path, m_x1 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC (), m_y0 + pTheme->GetArrowDist () / 2. + pTheme->GetArrowHeadC ());
		m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_bpath_ext_get_type (),
								"bpath", path,
								"outline_color", gcp::AddColor,
								"width_units", pTheme->GetArrowWidth (),
								"cap-style", GDK_CAP_BUTT,
								"join-style", GDK_JOIN_MITER,
								NULL);
		break;
	}
	}
	return true;
}

void gcpArrowTool::OnDrag ()
{
	double x1, y1, x2, y2;
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	if (m_pItem) {
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
		gtk_object_destroy(GTK_OBJECT (GNOME_CANVAS_ITEM (m_pItem)));
		gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
		m_pItem = NULL;
	}
	double dAngle;
	m_x -= m_x0;
	m_y -= m_y0;
	if (m_x == 0) {
		if (m_y == 0)
			return;
		dAngle = (m_y < 0) ? 90. : 270.;
	} else {
		dAngle = atan (-m_y / m_x) * 180. / M_PI;
		if (!(m_nState & GDK_CONTROL_MASK))
			dAngle = rint (dAngle / 5) * 5;
		if (m_x < 0)
			dAngle += 180;
	}
	m_dAngle = dAngle * M_PI / 180.;
	double d = (m_nState & GDK_SHIFT_MASK)? sqrt (square (m_x) + square (m_y)): pDoc->GetArrowLength () * pTheme->GetZoomFactor ();
	char tmp[32];
	if (dAngle < 0)
		dAngle += 360;
	snprintf (tmp, sizeof (tmp) - 1, _("Orientation: %g"), dAngle);
	m_pApp->SetStatusText (tmp);
	m_x1 = m_x0 + d * cos (m_dAngle);
	m_y1 = m_y0 - d * sin (m_dAngle);
	switch (m_ArrowType) {
	case gcp::SimpleArrow:
		points->coords[2] = m_x1;
		points->coords[3] = m_y1;
		m_pItem = gnome_canvas_item_new (
									m_pGroup,
									gnome_canvas_line_ext_get_type (),
									"points", points,
									"fill_color", gcp::AddColor,
									"width_units", pTheme->GetArrowWidth (),
									"last_arrowhead", true,
									"arrow_shape_a", pTheme->GetArrowHeadA (),
									"arrow_shape_b", pTheme->GetArrowHeadB (),
									"arrow_shape_c", pTheme->GetArrowHeadC (),
									"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
									NULL);
		break;
	case gcp::ReversibleArrow:
		points->coords[0] = m_x0 - pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[1] = m_y0 - pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		points->coords[2] = m_x1 - pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[3] = m_y1 - pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_LEFT,
							NULL);
		points->coords[2] = m_x0 + pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[3] = m_y0 + pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		points->coords[0] = m_x1 + pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[1] = m_y1 + pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_LEFT,
							NULL);
		break;
	case gcp::FullReversibleArrow:
		points->coords[0] = m_x0 - pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[1] = m_y0 - pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		points->coords[2] = m_x1 - pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[3] = m_y1 - pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		m_pItem = gnome_canvas_item_new (m_pGroup, gnome_canvas_group_ext_get_type (), NULL);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
							NULL);
		points->coords[2] = m_x0 + pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[3] = m_y0 + pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		points->coords[0] = m_x1 + pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		points->coords[1] = m_y1 + pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		gnome_canvas_item_new (
							GNOME_CANVAS_GROUP (m_pItem),
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", gcp::AddColor,
							"width_units", pTheme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a", pTheme->GetArrowHeadA (),
							"arrow_shape_b", pTheme->GetArrowHeadB (),
							"arrow_shape_c", pTheme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
							NULL);
		break;
	case gcpDoubleHeadedArrow:
		points->coords[2] = m_x1;
		points->coords[3] = m_y1;
		m_pItem = gnome_canvas_item_new (
									m_pGroup,
									gnome_canvas_line_ext_get_type (),
									"points", points,
									"fill_color", gcp::AddColor,
									"width_units", pTheme->GetArrowWidth (),
									"first_arrowhead", true,
									"last_arrowhead", true,
									"arrow_shape_a", pTheme->GetArrowHeadA (),
									"arrow_shape_b", pTheme->GetArrowHeadB (),
									"arrow_shape_c", pTheme->GetArrowHeadC (),
									"first_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
									"last_arrowhead_style", (unsigned char) ARROW_HEAD_BOTH,
									NULL);
		break;
	case gcpDoubleQueuedArrow: {
		double x1, y1;
		x1 = pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		y1 = pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		GnomeCanvasPathDef* path = gnome_canvas_path_def_new ();
		gnome_canvas_path_def_moveto (path, m_x0 - x1, m_y0 - y1);
		gnome_canvas_path_def_lineto (path, m_x1 - x1 - y1, m_y1 - y1 + x1);
		gnome_canvas_path_def_moveto (path, m_x0 + x1, m_y0 + y1);
		gnome_canvas_path_def_lineto (path, m_x1 + x1 - y1, m_y1 + y1 + x1);
		x1 += pTheme->GetArrowHeadC () * sin (m_dAngle);
		y1 += pTheme->GetArrowHeadC () * cos (m_dAngle);
		gnome_canvas_path_def_moveto (path, m_x1 - x1 - y1, m_y1 - y1 + x1);
		gnome_canvas_path_def_lineto (path, m_x1, m_y1);
		gnome_canvas_path_def_lineto (path, m_x1 + x1 - y1, m_y1 + y1 + x1);
		m_pItem = gnome_canvas_item_new (
								m_pGroup,
								gnome_canvas_bpath_ext_get_type (),
								"bpath", path,
								"outline_color", gcp::AddColor,
								"width_units", pTheme->GetArrowWidth (),
								"cap-style", GDK_CAP_BUTT,
								"join-style", GDK_JOIN_MITER,
								NULL);
		break;
	}
	}
}

void gcpArrowTool::OnRelease ()
{
	double x1, y1, x2, y2;
	if (m_pItem) {
		gnome_canvas_item_get_bounds (GNOME_CANVAS_ITEM (m_pItem), &x1, &y1, &x2, &y2);
		gtk_object_destroy (GTK_OBJECT (GNOME_CANVAS_ITEM (m_pItem)));
		gnome_canvas_request_redraw (GNOME_CANVAS (m_pWidget), (int) x1, (int) y1, (int) x2, (int) y2);
		m_pItem = NULL;
	}
	else return;
	m_pApp->ClearStatus ();
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcp::Arrow* a;
	switch (m_ArrowType)
	{
	case gcpDoubleHeadedArrow: {
			a = new gcp::MesomeryArrow (NULL);
			a->SetCoords (m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor, m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
			pDoc->AddObject (a);
		}
		break;
	case gcpDoubleQueuedArrow: {
			a = new gcpRetrosynthesisArrow (NULL);
			a->SetCoords (m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor, m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
			pDoc->AddObject (a);
		}
		break;
	default: {
			a = new gcp::ReactionArrow (NULL, m_ArrowType);
			a->SetCoords (m_x0 / m_dZoomFactor, m_y0 / m_dZoomFactor, m_x1 / m_dZoomFactor, m_y1 / m_dZoomFactor);
			pDoc->AddObject (a);
		}
	}
	pDoc->FinishOperation ();
}

static void on_full_toggled (GtkToggleButton *button, gcpArrowTool *tool)
{
	tool->SetArrowType (gtk_toggle_button_get_active (button)? gcp::FullReversibleArrow: gcp::ReversibleArrow);
}

static void on_length_changed (GtkSpinButton *btn, gcpArrowTool *tool)
{
	tool->SetLength (gtk_spin_button_get_value (btn));
}

GtkWidget *gcpArrowTool::GetPropertyPage ()
{
	bool show_all = m_ArrowType == gcp::ReversibleArrow || m_ArrowType == gcp::FullReversibleArrow;
	GladeXML *xml = glade_xml_new (GLADEDIR"/arrowtool.glade", (show_all? "arrow-box": "length-box"), GETTEXT_PACKAGE);
	if (show_all) {
		GtkTable *table = GTK_TABLE (glade_xml_get_widget (xml, "heads-table"));
		GnomeCanvas *canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
		gcp::Theme *Theme = gcp::TheThemeManager.GetTheme ("Default");
		double width = (Theme->GetArrowLength () * Theme->GetZoomFactor () + 2 * Theme->GetArrowPadding ()),
			height = Theme->GetArrowDist () + Theme->GetArrowWidth () + 2 * (Theme->GetArrowHeadB () + Theme->GetPadding ());
		gtk_widget_set_size_request (GTK_WIDGET (canvas), (int) width, (int) height);
		GnomeCanvasGroup *group = gnome_canvas_root (canvas);
		GnomeCanvasPoints *points = gnome_canvas_points_new (2);
		gnome_canvas_set_scroll_region (canvas, 0, 0, Theme->GetArrowLength (), Theme->GetArrowDist () + Theme->GetArrowWidth () + 2 * Theme->GetArrowHeadB ());
		points->coords[0] = (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		points->coords[1] = points->coords[3] = (height - Theme->GetArrowDist  ()) / 2.;
		points->coords[2] = (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		gnome_canvas_item_new (
							group,
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", "black",
							"width_units", Theme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a",Theme->GetArrowHeadA (),
							"arrow_shape_b",Theme->GetArrowHeadB (),
							"arrow_shape_c",Theme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_LEFT,
							NULL);
		points->coords[0] = (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		points->coords[1] = points->coords[3] = (height + Theme->GetArrowDist ()) / 2.;
		points->coords[2] = (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		gnome_canvas_item_new (
							group,
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", "black",
							"width_units",Theme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a",Theme->GetArrowHeadA (),
							"arrow_shape_b",Theme->GetArrowHeadB (),
							"arrow_shape_c",Theme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char) ARROW_HEAD_LEFT,
							NULL);
		gtk_widget_show (GTK_WIDGET (canvas));
		gtk_table_attach (table, GTK_WIDGET (canvas), 1, 2, 0, 1, GTK_FILL, GTK_FILL, 10, 0);
		canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
		gtk_widget_set_size_request (GTK_WIDGET (canvas), (int) width, (int) height);
		group = gnome_canvas_root (canvas);
		gnome_canvas_set_scroll_region (canvas, 0, 0, Theme->GetArrowLength (), Theme->GetArrowDist () + Theme->GetArrowWidth () + 2 * Theme->GetArrowHeadB ());
		points->coords[0] = (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		points->coords[1] = points->coords[3] = (height - Theme->GetArrowDist ()) / 2.;
		points->coords[2] = (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		gnome_canvas_item_new (
							group,
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", "black",
							"width_units", Theme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a",Theme->GetArrowHeadA (),
							"arrow_shape_b",Theme->GetArrowHeadB (),
							"arrow_shape_c",Theme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_BOTH,
							NULL);
		points->coords[0] = (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		points->coords[1] = points->coords[3] = (height + Theme->GetArrowDist ()) / 2.;
		points->coords[2] = (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.;
		gnome_canvas_item_new (
							group,
							gnome_canvas_line_ext_get_type (),
							"points", points,
							"fill_color", "black",
							"width_units",Theme->GetArrowWidth (),
							"last_arrowhead", true,
							"arrow_shape_a",Theme->GetArrowHeadA (),
							"arrow_shape_b",Theme->GetArrowHeadB (),
							"arrow_shape_c",Theme->GetArrowHeadC (),
							"last_arrowhead_style", (unsigned char)ARROW_HEAD_BOTH,
							NULL);
		gtk_widget_show (GTK_WIDGET (canvas));
		gtk_table_attach (table, GTK_WIDGET (canvas), 1, 2, 1, 2, GTK_FILL, GTK_FILL, 10, 0);
		gnome_canvas_points_free (points);
		GtkWidget *b = glade_xml_get_widget (xml, "full");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), m_ArrowType == gcp::FullReversibleArrow);
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (on_full_toggled), this);
		GtkWidget *w = glade_xml_get_widget (xml, "default");
		g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_default), b);
	}
	m_LengthBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "arrow-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	return glade_xml_get_widget (xml, (show_all? "arrow-box": "length-box"));
}

void gcpArrowTool::SetLength (double length)
{
	m_pApp->GetActiveDocument ()->SetArrowLength (length);
}

void gcpArrowTool::Activate ()
{
	gtk_spin_button_set_value (m_LengthBtn, m_pApp->GetActiveDocument ()->GetArrowLength ());
}
