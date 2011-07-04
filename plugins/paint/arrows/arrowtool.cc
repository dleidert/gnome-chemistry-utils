// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * arrowtool.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <gcp/document.h>
#include <gcp/application.h>
#include <gcp/mesomery-arrow.h>
#include <gcp/reaction-arrow.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gccv/arrow.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/line.h>
#include <gccv/poly-line.h>
#include <gcugtk/ui-builder.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace std;

static char const *ToolNames[] = {
	"SimpleArrow",
	"ReversibleArrow",
	"ReversibleArrow",
	"DoubleHeadedArrow",
	"RetrosynthesisArrow"
};

static void on_default (GtkToggleButton *button)
{
	GOConfNode *node = go_conf_get_node (gcu::Application::GetConfDir (), "paint/plugins/arrows");
	go_conf_set_bool (node, "full-arrows-heads", gtk_toggle_button_get_active (button));
	go_conf_free_node (node);
}

gcpArrowTool::gcpArrowTool (gcp::Application* App, unsigned ArrowType): gcp::Tool (App, ToolNames[ArrowType])
{
	m_ArrowType = ArrowType;
}

gcpArrowTool::~gcpArrowTool ()
{
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
	case gcp::SimpleArrow: {
		gccv::Arrow *arrow = new gccv::Arrow (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		m_Item = arrow;
		break;
	}
	case gcp::ReversibleArrow: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Arrow *arrow = new gccv::Arrow (group, m_x0, m_y0 - pTheme->GetArrowDist () / 2, m_x1, m_y1 - pTheme->GetArrowDist () / 2, NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow = new gccv::Arrow (group, m_x1, m_y1 + pTheme->GetArrowDist () / 2, m_x0, m_y0 + pTheme->GetArrowDist () / 2, NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		m_Item = group;
		break;
	}
	case gcp::FullReversibleArrow: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Arrow *arrow = new gccv::Arrow (group, m_x0, m_y0 - pTheme->GetArrowDist () / 2, m_x1, m_y1 - pTheme->GetArrowDist () / 2, NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow = new gccv::Arrow (group, m_x1, m_y1 + pTheme->GetArrowDist () / 2, m_x0, m_y0 + pTheme->GetArrowDist () / 2, NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		m_Item = group;
		break;
	}
	case gcpDoubleHeadedArrow: {
		gccv::Arrow *arrow = new gccv::Arrow (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetStartHead (gccv::ArrowHeadFull);
		m_Item = arrow;
		break;
	}
	case gcpDoubleQueuedArrow: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Line *line = new gccv::Line (group, m_x0,
										   m_y0 - pTheme->GetArrowDist () / 2.,
										   m_x1 - pTheme->GetArrowDist () / 2.,
										   m_y0 - pTheme->GetArrowDist () / 2.,
										   NULL);
		line->SetLineColor (gcp::AddColor);
		line->SetLineWidth (pTheme->GetArrowWidth ());
		line = new gccv::Line (group, m_x0,
							   m_y0 + pTheme->GetArrowDist () / 2.,
							   m_x1 - pTheme->GetArrowDist () / 2.,
							   m_y0 + pTheme->GetArrowDist () / 2.,
							   NULL);
		line->SetLineColor (gcp::AddColor);
		line->SetLineWidth (pTheme->GetArrowWidth ());
		list <gccv::Point> points;
		gccv::Point point;
		point.x = m_x1 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC ();
		point.y = m_y0 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC ();
		points.push_back (point);
		point.x = m_x1;
		point.y = m_y0;
		points.push_back (point);
		point.x = m_x1 - pTheme->GetArrowDist () / 2. - pTheme->GetArrowHeadC ();
		point.y = m_y0 + pTheme->GetArrowDist () / 2. + pTheme->GetArrowHeadC ();
		points.push_back (point);
		gccv::PolyLine *pl = new gccv::PolyLine (group, points, NULL);
		pl->SetLineColor (gcp::AddColor);
		pl->SetLineWidth (pTheme->GetArrowWidth ());
		m_Item = group;
		break;
	}
	}
	return true;
}

void gcpArrowTool::OnDrag ()
{
	gcp::Document *pDoc = m_pView->GetDoc ();
	gcp::Theme *pTheme = pDoc->GetTheme ();
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
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
	case gcp::SimpleArrow: {
		gccv::Arrow *arrow = new gccv::Arrow (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		m_Item = arrow;
		break;
	}
	case gcp::ReversibleArrow: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x0 - pTheme->GetArrowDist () / 2. * sin (m_dAngle),
											  m_y0 - pTheme->GetArrowDist () / 2. * cos (m_dAngle),
											  m_x1 - pTheme->GetArrowDist () / 2. * sin (m_dAngle),
											  m_y1 - pTheme->GetArrowDist () / 2. * cos (m_dAngle),
											  NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow = new gccv::Arrow (group,
								 m_x1 + pTheme->GetArrowDist () / 2. * sin (m_dAngle),
								 m_y1 + pTheme->GetArrowDist () / 2. * cos (m_dAngle),
								 m_x0 + pTheme->GetArrowDist () / 2. * sin (m_dAngle),
								 m_y0 + pTheme->GetArrowDist () / 2. * cos (m_dAngle),
								 NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		m_Item = group;
		break;
	}
	case gcp::FullReversibleArrow: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Arrow *arrow = new gccv::Arrow (group,
											  m_x0 - pTheme->GetArrowDist () / 2. * sin (m_dAngle),
											  m_y0 - pTheme->GetArrowDist () / 2. * cos (m_dAngle),
											  m_x1 - pTheme->GetArrowDist () / 2. * sin (m_dAngle),
											  m_y1 - pTheme->GetArrowDist () / 2. * cos (m_dAngle),
											  NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow = new gccv::Arrow (group,
								 m_x1 + pTheme->GetArrowDist () / 2. * sin (m_dAngle),
								 m_y1 + pTheme->GetArrowDist () / 2. * cos (m_dAngle),
								 m_x0 + pTheme->GetArrowDist () / 2. * sin (m_dAngle),
								 m_y0 + pTheme->GetArrowDist () / 2. * cos (m_dAngle),
								 NULL);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		m_Item = group;
		break;
	}
	case gcpDoubleHeadedArrow: {
		gccv::Arrow *arrow = new gccv::Arrow (m_pView->GetCanvas (), m_x0, m_y0, m_x1, m_y1);
		arrow->SetLineColor (gcp::AddColor);
		arrow->SetLineWidth (pTheme->GetArrowWidth ());
		arrow->SetA (pTheme->GetArrowHeadA ());
		arrow->SetB (pTheme->GetArrowHeadB ());
		arrow->SetC (pTheme->GetArrowHeadC ());
		arrow->SetStartHead (gccv::ArrowHeadFull);
		m_Item = arrow;
		break;
	}
	case gcpDoubleQueuedArrow: {
		double x1, y1;
		x1 = pTheme->GetArrowDist () / 2 * sin (m_dAngle);
		y1 = pTheme->GetArrowDist () / 2 * cos (m_dAngle);
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Line *line = new gccv::Line (group, m_x0 - x1, m_y0 - y1,
										   m_x1- x1 - y1, m_y1 - y1 + x1,
										   NULL);
		line->SetLineColor (gcp::AddColor);
		line->SetLineWidth (pTheme->GetArrowWidth ());
		line = new gccv::Line (group,m_x0 + x1, m_y0 + y1,
							   m_x1 + x1 - y1, m_y1 + y1 + x1,
							   NULL);
		line->SetLineColor (gcp::AddColor);
		line->SetLineWidth (pTheme->GetArrowWidth ());
		x1 += pTheme->GetArrowHeadC () * sin (m_dAngle);
		y1 += pTheme->GetArrowHeadC () * cos (m_dAngle);
		list <gccv::Point> points;
		gccv::Point point;
		point.x = m_x1 - x1 - y1;
		point.y = m_y1 - y1 + x1;
		points.push_back (point);
		point.x = m_x1;
		point.y = m_y1;
		points.push_back (point);
		point.x = m_x1 + x1 - y1;
		point.y = m_y1 + y1 + x1;
		points.push_back (point);
		gccv::PolyLine *pl = new gccv::PolyLine (group, points, NULL);
		pl->SetLineColor (gcp::AddColor);
		pl->SetLineWidth (pTheme->GetArrowWidth ());
		m_Item = group;
		break;
	}
	}
}

void gcpArrowTool::OnRelease ()
{
	if (m_Item) {
		delete m_Item;
		m_Item = NULL;
	}
	else
		return;
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
	gcp::Operation *op = pDoc->GetNewOperation (gcp::GCP_ADD_OPERATION);
	op->AddObject (a);
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
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/arrowtool.ui", GETTEXT_PACKAGE);
	GtkWidget *res = builder->GetRefdWidget ("arrow-grid");
	if (show_all) {
		GtkGrid *grid = GTK_GRID (res);
		gccv::Canvas *canvas = new gccv::Canvas (NULL);
		gcp::Theme *Theme = gcp::TheThemeManager.GetTheme ("Default");
		double width = (Theme->GetArrowLength () * Theme->GetZoomFactor () + 2 * Theme->GetArrowPadding ()),
			height = Theme->GetArrowDist () + Theme->GetArrowWidth () + 2 * (Theme->GetArrowHeadB () + Theme->GetPadding ());
		gtk_widget_set_size_request (canvas->GetWidget (), (int) width, (int) height);
		gccv::Arrow *arrow = new gccv::Arrow (canvas,
											  (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
											  (height - Theme->GetArrowDist  ()) / 2.,
											  (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
											  (height - Theme->GetArrowDist  ()) / 2.);
		arrow->SetLineWidth (Theme->GetArrowWidth ());
		arrow->SetA (Theme->GetArrowHeadA ());
		arrow->SetB (Theme->GetArrowHeadB ());
		arrow->SetC (Theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow->SetAutoColor (true);
		arrow = new gccv::Arrow (canvas,
								 (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height + Theme->GetArrowDist  ()) / 2.,
								 (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height + Theme->GetArrowDist  ()) / 2.);
		arrow->SetLineWidth (Theme->GetArrowWidth ());
		arrow->SetA (Theme->GetArrowHeadA ());
		arrow->SetB (Theme->GetArrowHeadB ());
		arrow->SetC (Theme->GetArrowHeadC ());
		arrow->SetEndHead (gccv::ArrowHeadLeft);
		arrow->SetAutoColor (true);
		gtk_widget_show (canvas->GetWidget ());
		gtk_grid_attach (grid, canvas->GetWidget (), 1, 2, 1, 1);
		canvas = new gccv::Canvas (NULL);
		arrow = new gccv::Arrow (canvas,
								 (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height - Theme->GetArrowDist  ()) / 2.,
								 (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height - Theme->GetArrowDist  ()) / 2.);
		arrow->SetLineWidth (Theme->GetArrowWidth ());
		arrow->SetA (Theme->GetArrowHeadA ());
		arrow->SetB (Theme->GetArrowHeadB ());
		arrow->SetC (Theme->GetArrowHeadC ());
		arrow->SetAutoColor (true);
		arrow = new gccv::Arrow (canvas,
								 (width + Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height + Theme->GetArrowDist  ()) / 2.,
								 (width - Theme->GetArrowLength () * Theme->GetZoomFactor ()) / 2.,
								 (height + Theme->GetArrowDist  ()) / 2.);
		arrow->SetLineWidth (Theme->GetArrowWidth ());
		arrow->SetA (Theme->GetArrowHeadA ());
		arrow->SetB (Theme->GetArrowHeadB ());
		arrow->SetC (Theme->GetArrowHeadC ());
		arrow->SetAutoColor (true);
		gtk_widget_show (canvas->GetWidget ());
		gtk_grid_attach (grid, canvas->GetWidget (), 1, 3, 1, 1);
		GtkWidget *b = builder->GetWidget ("full");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b), m_ArrowType == gcp::FullReversibleArrow);
		g_signal_connect (G_OBJECT (b), "toggled", G_CALLBACK (on_full_toggled), this);
		GtkWidget *w = builder->GetWidget ("default");
		g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_default), b);
	} else {
		gtk_widget_hide (builder->GetWidget ("half"));
		gtk_widget_hide (builder->GetWidget ("full"));
		gtk_widget_hide (builder->GetWidget ("arrow-sep"));
		gtk_widget_hide (builder->GetWidget ("default"));
	}
	m_LengthBtn = GTK_SPIN_BUTTON (builder->GetWidget ("arrow-length"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (on_length_changed), this);
	delete builder;
	return res;
}

void gcpArrowTool::SetLength (double length)
{
	m_pApp->GetActiveDocument ()->SetArrowLength (length);
}

void gcpArrowTool::Activate ()
{
	gtk_spin_button_set_value (m_LengthBtn, m_pApp->GetActiveDocument ()->GetArrowLength ());
}
