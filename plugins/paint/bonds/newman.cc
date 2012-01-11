// -*- C++ -*-

/*
 * GChemPaint bonds plugin
 * newman.cc
 *
 * Copyright (C) 2012 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "newman.h"
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcugtk/ui-builder.h>
#include <gccv/canvas.h>
#include <gccv/circle.h>
#include <gccv/group.h>
#include <gccv/line.h>

class gcpNewmanToolPrivate
{
public:
	static void OnLengthChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnOrderChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnForeBondsChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnRearBondsChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnForeFirstAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnRearFirstAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnForeBondAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
	static void OnRearBondAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool);
};

void gcpNewmanToolPrivate::OnLengthChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_pApp->GetActiveDocument ()->SetBondLength (gtk_spin_button_get_value (btn));
}

void gcpNewmanToolPrivate::OnOrderChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_Order = gtk_spin_button_get_value_as_int (btn);
	switch (tool->m_Order) {
	case 1:
		gtk_spin_button_set_value (tool->m_ForeBondsBtn, 3);
		gtk_spin_button_set_value (tool->m_RearBondsBtn, 3);
		gtk_spin_button_set_value (tool->m_RearFirstAngleBtn, tool->m_ForeFirstAngle * 180. / M_PI - 180.);
		break;
	case 2:
		gtk_spin_button_set_value (tool->m_ForeBondsBtn, 2);
		gtk_spin_button_set_value (tool->m_RearBondsBtn, 2);
		gtk_spin_button_set_value (tool->m_RearFirstAngleBtn, tool->m_ForeFirstAngle * 180. / M_PI);
		break;
	}
}

void gcpNewmanToolPrivate::OnForeBondsChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_ForeBonds = gtk_spin_button_get_value_as_int (btn);
	switch (tool->m_ForeBonds) {
	case 2:
		gtk_spin_button_set_value (tool->m_ForeBondAngleBtn, 180.);
		break;
	case 3:
		gtk_spin_button_set_value (tool->m_ForeBondAngleBtn, 120.);
		break;
	}
}

void gcpNewmanToolPrivate::OnRearBondsChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_RearBonds = gtk_spin_button_get_value_as_int (btn);
	switch (tool->m_RearBonds) {
	case 2:
		gtk_spin_button_set_value (tool->m_RearBondAngleBtn, 180.);
		break;
	case 3:
		gtk_spin_button_set_value (tool->m_RearBondAngleBtn, 120.);
		break;
	}
}

void gcpNewmanToolPrivate::OnForeFirstAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_ForeFirstAngle = gtk_spin_button_get_value_as_int (btn) * M_PI / 180.;
}

void gcpNewmanToolPrivate::OnForeBondAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_ForeBondAngle = gtk_spin_button_get_value_as_int (btn) * M_PI / 180.;
}

void gcpNewmanToolPrivate::OnRearFirstAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_RearFirstAngle = gtk_spin_button_get_value_as_int (btn) * M_PI / 180.;
}

void gcpNewmanToolPrivate::OnRearBondAngleChanged (GtkSpinButton *btn, gcpNewmanTool *tool)
{
	tool->m_RearBondAngle = gtk_spin_button_get_value_as_int (btn) * M_PI / 180.;
}

gcpNewmanTool::gcpNewmanTool (gcp::Application *App): gcp::Tool (App, "Newman")
{
	m_ForeBonds = m_RearBonds = 3;
	m_ForeFirstAngle = M_PI / 2.;
	m_RearFirstAngle = -M_PI / 2.;
	m_ForeBondAngle = m_RearBondAngle = M_PI / 1.5;
}

gcpNewmanTool::~gcpNewmanTool ()
{
}

bool gcpNewmanTool::OnClicked ()
{
	if (m_pObject != NULL)
		return false;
	// create the item
	gcp::Document* doc = m_pView->GetDoc ();
	double length = doc->GetBondLength () * m_dZoomFactor;
	double angle;
	int i;
	gccv::Line *line;
	gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
	m_Item = group;
	gccv::Circle *circle = new gccv::Circle (group, m_x0, m_y0, length / 3., NULL);
	circle->SetFillColor (0);
	circle->SetLineColor (GO_COLOR_BLACK);
	angle = m_ForeFirstAngle;
	for (i = 0; i < m_ForeBonds; i++) {
		line = new gccv::Line (group, m_x0, m_y0, m_x0 + length * cos (angle), m_y0 - length * sin (angle), NULL);
		line->SetLineColor (GO_COLOR_BLACK);
		angle += m_ForeBondAngle;
	}
	angle = m_RearFirstAngle;
	for (i = 0; i < m_RearBonds; i++) {
		line = new gccv::Line (group, m_x0 + length / 3. * cos (angle),
		                       m_y0 - length / 3. * sin (angle),
		                       m_x0 + length * cos (angle),
		                       m_y0 - length * sin (angle), NULL);
		line->SetLineColor (GO_COLOR_BLACK);
		angle += m_RearBondAngle;
	}
	return true;
}

void gcpNewmanTool::OnDrag ()
{
	delete m_Item;
	gcp::Document* doc = m_pView->GetDoc ();
	double length = doc->GetBondLength () * m_dZoomFactor;
	double angle;
	int i;
	gccv::Line *line;
	gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
	m_Item = group;
	gccv::Circle *circle = new gccv::Circle (group, m_x, m_y, length / 3., NULL);
	circle->SetFillColor (0);
	circle->SetLineColor (GO_COLOR_BLACK);
	angle = m_ForeFirstAngle;
	for (i = 0; i < m_ForeBonds; i++) {
		line = new gccv::Line (group, m_x, m_y, m_x + length * cos (angle), m_y - length * sin (angle), NULL);
		line->SetLineColor (GO_COLOR_BLACK);
		angle += m_ForeBondAngle;
	}
	angle = m_RearFirstAngle;
	for (i = 0; i < m_RearBonds; i++) {
		line = new gccv::Line (group, m_x + length / 3. * cos (angle),
		                       m_y - length / 3. * sin (angle),
		                       m_x + length * cos (angle),
		                       m_y - length * sin (angle), NULL);
		line->SetLineColor (GO_COLOR_BLACK);
		angle += m_RearBondAngle;
	}
}

void gcpNewmanTool::OnRelease ()
{
	delete m_Item;
	m_Item = NULL;
	gcp::Document* doc = m_pView->GetDoc ();
	double bl = doc->GetBondLength (), dz = bl / 2.;
	double angle;
	int i;
	// add the atoms and bonds
	// for now, we only add carbon atoms, and never merge with existing atoms
	// using 3d coordinates, front atoms at +bond_length/2 rear atoms at -bond_length/2
	m_x /= m_dZoomFactor;
	m_y /= m_dZoomFactor;
	gcp::Atom *atom = new gcp::Atom (6, m_x, m_y, -bl / 2.), *atom0;
	doc->AddAtom (atom);
	angle = m_RearFirstAngle;
	for (i = 0; i < m_RearBonds; i++) {
		atom0 = new gcp::Atom (6, m_x + bl * cos (angle), m_y - bl * sin (angle), -dz);
		doc->AddAtom (atom0);
		doc->AddBond (new gcp::Bond (atom, atom0, 1));
		angle += m_RearBondAngle;
	}
	atom0 = new gcp::Atom (6, m_x, m_y, dz);
	doc->AddAtom (atom0);
	gcp::Bond *bond = new gcp::Bond (atom, atom0, 1);
	doc->AddBond (bond); // add to doc before setting the type so that the radius will be correct
	bond->SetType (gcp::NewmanBondType);
	angle = m_ForeFirstAngle;
	for (i = 0; i < m_ForeBonds; i++) {
		atom = new gcp::Atom (6, m_x + bl * cos (angle), m_y - bl * sin (angle), -dz);
		doc->AddAtom (atom);
		doc->AddBond (new gcp::Bond (atom0, atom, 1));
		angle += m_ForeBondAngle;
	}
	gcp::Operation *op = doc-> GetNewOperation (gcp::GCP_ADD_OPERATION);
	op->AddObject (bond->GetMolecule ());
	doc->FinishOperation ();
	// ensure that rear bonds don't go up to the center
	m_pView->Update (bond->GetMolecule ());
}

GtkWidget *gcpNewmanTool::GetPropertyPage ()
{
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/newman.ui", GETTEXT_PACKAGE);
	m_LengthBtn = GTK_SPIN_BUTTON (builder->GetWidget ("bond-length-btn"));
	g_signal_connect (m_LengthBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnLengthChanged), this);
	m_OrderBtn = GTK_SPIN_BUTTON (builder->GetWidget ("order-btn"));
	gtk_spin_button_set_value (m_OrderBtn, m_Order);
	g_signal_connect (m_OrderBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnOrderChanged), this);
	m_ForeBondsBtn = GTK_SPIN_BUTTON (builder->GetWidget ("fore-bonds-btn"));
	gtk_spin_button_set_value (m_ForeBondsBtn, m_ForeBonds);
	g_signal_connect (m_ForeBondsBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnForeBondsChanged), this);
	m_RearBondsBtn = GTK_SPIN_BUTTON (builder->GetWidget ("rear-bonds-btn"));
	gtk_spin_button_set_value (m_RearBondsBtn, m_RearBonds);
	g_signal_connect (m_RearBondsBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnRearBondsChanged), this);
	m_ForeFirstAngleBtn = GTK_SPIN_BUTTON (builder->GetWidget ("fore-first-angle-btn"));
	gtk_spin_button_set_value (m_ForeFirstAngleBtn, m_ForeFirstAngle * 180. / M_PI);
	g_signal_connect (m_ForeFirstAngleBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnForeFirstAngleChanged), this);
	m_RearFirstAngleBtn = GTK_SPIN_BUTTON (builder->GetWidget ("rear-first-angle-btn"));
	gtk_spin_button_set_value (m_RearFirstAngleBtn, m_RearFirstAngle * 180. / M_PI);
	g_signal_connect (m_RearFirstAngleBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnRearFirstAngleChanged), this);
	m_ForeBondAngleBtn = GTK_SPIN_BUTTON (builder->GetWidget ("fore-angle-btn"));
	gtk_spin_button_set_value (m_ForeBondAngleBtn, m_ForeBondAngle * 180. / M_PI);
	g_signal_connect (m_ForeBondAngleBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnForeBondAngleChanged), this);
	m_RearBondAngleBtn = GTK_SPIN_BUTTON (builder->GetWidget ("rear-angle-btn"));
	gtk_spin_button_set_value (m_RearBondAngleBtn, m_RearBondAngle * 180. / M_PI);
	g_signal_connect (m_RearBondAngleBtn, "value-changed", G_CALLBACK (gcpNewmanToolPrivate::OnRearBondAngleChanged), this);
	// FIXME: implement
	GtkWidget *res = builder->GetRefdWidget ("newman");
	delete builder;
	return res;
}

void gcpNewmanTool::Activate ()
{
	gcp::Document *pDoc = m_pApp->GetActiveDocument ();
	gtk_spin_button_set_value (m_LengthBtn, pDoc->GetBondLength ());
}
