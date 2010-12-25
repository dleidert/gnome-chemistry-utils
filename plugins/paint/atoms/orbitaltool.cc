// -*- C++ -*-

/* 
 * GChemPaint atoms plugin
 * orbitaltool.cc 
 *
 * Copyright (C) 2009-2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "orbitaltool.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/document.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/leaf.h>
#include <gcu/ui-builder.h>

gcpOrbitalTool::gcpOrbitalTool (gcp::Application *App):
	gcp::Tool (App, "Orbital"),
	m_Coef (1.),
	m_Rotation (0.),
	m_Type (GCP_ORBITAL_TYPE_S),
	m_PreviewItem (NULL)
{
}

gcpOrbitalTool::~gcpOrbitalTool ()
{
}

bool gcpOrbitalTool::OnClicked ()
{
	if (!m_pObject || (m_pObject->GetType () != gcu::AtomType))
		return false;
	m_pData->UnselectAll ();
	gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
	// FIXME: don't allow two orbitals for the same atom.
	atom->GetCoords (&m_x0, &m_y0);
	gcp::Document *doc = m_pView->GetDoc ();
	gcp::Theme *theme = doc->GetTheme ();
	m_x0 *= m_dZoomFactor;
	m_y0 *= m_dZoomFactor;
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S: {
		gccv::Circle *circle = new gccv::Circle (m_pView->GetCanvas (), m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor / 2.);
		circle->SetLineWidth (1.);
		circle->SetLineColor (gcp::AddColor);
		circle->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		m_Item = circle;
		break;
	}
	case GCP_ORBITAL_TYPE_P: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Leaf *leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI + M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (GO_COLOR_WHITE);
		m_Item = group;
		break;
	}
	case GCP_ORBITAL_TYPE_DXY: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Leaf *leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = group;
		break;
	}
		break;
	case GCP_ORBITAL_TYPE_DZ2: {
		gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
		gccv::Leaf *leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, m_x0, m_y0, theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * m_dZoomFactor);
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::AddColor);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_Item = group;
		break;
	}
	default:
		break;
	}
	return true;
}

void gcpOrbitalTool::OnDrag ()
{
	if (!m_Item)
		return;
	gccv::Item *item = dynamic_cast <gccv::ItemClient *> (m_pObject)->GetItem ();
	double x0, y0, x1, y1;
	item->GetBounds (x0, y0, x1, y1);
	m_Item->SetVisible (m_x >= x0 && m_x <= x1 && m_y >= y0 && m_y <= y1);
}

void gcpOrbitalTool::OnRelease ()
{
	if (!m_Item || !m_Item->GetVisible ())
		return;
	gcp::Atom *atom = static_cast <gcp::Atom *> (m_pObject);
	gcu::Object *obj = m_pObject->GetGroup ();
	gcp::Document* doc = m_pView->GetDoc ();
	gcp::Operation* op = doc-> GetNewOperation (gcp::GCP_MODIFY_OPERATION);
	op->AddObject (obj, 0);
	gcpOrbital *orbital = new gcpOrbital (atom, m_Type);
	orbital->SetCoef (m_Coef);
	orbital->SetRotation (m_Rotation);
	m_pObject->EmitSignal (gcp::OnChangedSignal);
	op->AddObject (obj, 1);
	doc->FinishOperation ();
	m_pView->AddObject (orbital);
}

void gcpOrbitalTool::OnMotion ()
{
	m_pData->UnselectAll ();
	bool allowed = false;
	if (m_pObject)
		switch (m_pObject->GetType ()) {
		case gcu::AtomType:
			allowed = true;
			break;
		default:
			break;
		}
	if (allowed)
		m_pData->SetSelected (m_pObject);
	gdk_window_set_cursor (gtk_widget_get_parent_window (m_pWidget), allowed? m_pApp->GetCursor (gcp::CursorPencil): m_pApp->GetCursor (gcp::CursorUnallowed));
}

void gcpOrbitalTool::OnLeaveNotify ()
{
	m_pData->UnselectAll ();
}

GtkWidget *gcpOrbitalTool::GetPropertyPage ()
{
	gcu::UIBuilder *builder = new gcu::UIBuilder (UIDIR"/orbital.ui", GETTEXT_PACKAGE);
	m_CoefBtn = GTK_SPIN_BUTTON (builder->GetWidget ("coef-btn"));
	gtk_spin_button_set_value (m_CoefBtn, m_Coef);
	g_signal_connect_swapped (m_CoefBtn, "value-changed", G_CALLBACK (CoefChanged), this);
	m_RotationLbl = builder->GetWidget ("rotation-lbl");
	m_RotationBtn = GTK_SPIN_BUTTON (builder->GetWidget ("rotation-btn"));
	gtk_widget_set_sensitive (m_RotationLbl, m_Type != GCP_ORBITAL_TYPE_S);
	gtk_spin_button_set_value (m_RotationBtn, m_Rotation);
	gtk_widget_set_sensitive (GTK_WIDGET (m_RotationBtn), m_Type != GCP_ORBITAL_TYPE_S);
	g_signal_connect_swapped (m_RotationBtn, "value-changed", G_CALLBACK (RotationChanged), this);
	GtkWidget *w = builder->GetWidget ("s-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_S));
	g_signal_connect_swapped (G_OBJECT (w), "toggled", G_CALLBACK (TypeChanged), this);
	w = builder->GetWidget ("p-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_P));
	g_signal_connect_swapped (G_OBJECT (w), "toggled", G_CALLBACK (TypeChanged), this);
	w = builder->GetWidget ("dxy-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_DXY));
	g_signal_connect_swapped (G_OBJECT (w), "toggled", G_CALLBACK (TypeChanged), this);
	w = builder->GetWidget ("dz2-btn");
	g_object_set_data (G_OBJECT (w), "orbital-type", GUINT_TO_POINTER (GCP_ORBITAL_TYPE_DZ2));
	g_signal_connect_swapped (G_OBJECT (w), "toggled", G_CALLBACK (TypeChanged), this);
	GtkWidget *res = builder->GetRefdWidget ("orbital");
	m_Preview = new gccv::Canvas (NULL);
	w = m_Preview->GetWidget ();
	gtk_widget_show (w);
	gtk_table_attach (GTK_TABLE (res), w, 1, 3, 3, 8, GTK_FILL, GTK_FILL, 10, 0);
	g_signal_connect_swapped (G_OBJECT (w), "size-allocate", G_CALLBACK (SizeAllocate), this);
	delete builder;
	return res;
}

void gcpOrbitalTool::CoefChanged (gcpOrbitalTool *tool, GtkSpinButton *btn)
{
	tool->m_Coef = gtk_spin_button_get_value (btn);
	tool->UpdatePreview ();
}

void gcpOrbitalTool::TypeChanged (gcpOrbitalTool *tool, GtkToggleButton *btn)
{
	if (gtk_toggle_button_get_active (btn))
		tool->m_Type = static_cast <gcpOrbitalType> (GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (btn), "orbital-type")));
	gtk_widget_set_sensitive (GTK_WIDGET (tool->m_RotationBtn), tool->m_Type != GCP_ORBITAL_TYPE_S);
	gtk_widget_set_sensitive (tool->m_RotationLbl, tool->m_Type != GCP_ORBITAL_TYPE_S);
	tool->UpdatePreview ();
}

void gcpOrbitalTool::RotationChanged (gcpOrbitalTool *tool, GtkSpinButton *btn)
{
	tool->m_Rotation = gtk_spin_button_get_value (btn);
	tool->UpdatePreview ();
}

void gcpOrbitalTool::SizeAllocate (gcpOrbitalTool *tool)
{
	tool->UpdatePreview ();
}

void gcpOrbitalTool::UpdatePreview ()
{
	if (m_PreviewItem)
		delete m_PreviewItem;
	gcp::Theme *theme = gcp::TheThemeManager.GetTheme ("Default");
	GtkAllocation alloc;
	gtk_widget_get_allocation (m_Preview->GetWidget (), &alloc);
	double x = alloc.width / 2;
	double y = alloc.height / 2;
	switch (m_Type) {
	case GCP_ORBITAL_TYPE_S: {
		gccv::Circle *circle = new gccv::Circle (m_Preview, x,  y, theme->GetBondLength () * m_Coef * theme->GetZoomFactor () / 2.);
		circle->SetLineWidth (1.);
		circle->SetLineColor (gcp::Color);
		circle->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		m_PreviewItem = circle;
		break;
	}
	case GCP_ORBITAL_TYPE_P: {
		gccv::Group *group = new gccv::Group (m_Preview,  x,  y);
		gccv::Leaf *leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_P_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI + M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (GO_COLOR_WHITE);
		m_PreviewItem = group;
		break;
	}
	case GCP_ORBITAL_TYPE_DXY: {
		gccv::Group *group = new gccv::Group (m_Preview,  x,  y);
		gccv::Leaf *leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.25) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.75) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_PreviewItem = group;
		break;
	}
		break;
	case GCP_ORBITAL_TYPE_DZ2: {
		gccv::Group *group = new gccv::Group (m_Preview, x, y);
		gccv::Leaf *leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation (m_Rotation / 180. * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_GREY (100): GO_COLOR_WHITE);
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + .5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		leaf = new gccv::Leaf (group, 0., 0., theme->GetBondLength () * m_Coef * GCP_ORBITAL_DZ2_FACTOR * theme->GetZoomFactor ());
		leaf->SetWidthFactor (GCP_ORBITAL_D_WIDTH);
		leaf->SetRotation ((m_Rotation / 180. + 1.5) * M_PI);
		leaf->SetLineWidth (1.);
		leaf->SetLineColor (gcp::Color);
		leaf->SetFillColor (m_Coef > 0.? GO_COLOR_WHITE: GO_COLOR_GREY (100));
		m_PreviewItem = group;
		break;
	}
	default:
		break;
	}
}
