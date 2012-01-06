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

gcpNewmanTool::gcpNewmanTool (gcp::Application *App): gcp::Tool (App, "Newman")
{
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
	gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
	m_Item = group;
	gccv::Circle *circle = new gccv::Circle (group, m_x0, m_y0, length / 3., NULL);
	circle->SetFillColor (0);
	circle->SetLineColor (GO_COLOR_BLACK);
	gccv::Line *line = new gccv::Line (group, m_x0, m_y0, m_x0, m_y0 - length, NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x0, m_y0, m_x0 + length * sqrt (3.) / 2., m_y0 + length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x0, m_y0, m_x0 - length * sqrt (3.) / 2., m_y0 + length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x0, m_y0 + length / 3., m_x0, m_y0 + length, NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x0 + length * sqrt (3.) / 6., m_y0 - length / 6.,
	                       m_x0 + length * sqrt (3.) / 2., m_y0 - length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x0 - length * sqrt (3.) / 6., m_y0 - length / 6.,
	                       m_x0 - length * sqrt (3.) / 2., m_y0 - length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	return true;
}

void gcpNewmanTool::OnDrag ()
{
	delete m_Item;
	gcp::Document* doc = m_pView->GetDoc ();
	double length = doc->GetBondLength () * m_dZoomFactor;
	gccv::Group *group = new gccv::Group (m_pView->GetCanvas ());
	m_Item = group;
	gccv::Circle *circle = new gccv::Circle (group, m_x, m_y, length / 3., NULL);
	circle->SetFillColor (0);
	circle->SetLineColor (GO_COLOR_BLACK);
	gccv::Line *line = new gccv::Line (group, m_x, m_y, m_x, m_y - length, NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x, m_y, m_x + length * sqrt (3.) / 2., m_y + length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x, m_y, m_x - length * sqrt (3.) / 2., m_y + length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x, m_y + length / 4., m_x, m_y + length, NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x + length * sqrt (3.) / 6., m_y - length / 6.,
	                       m_x + length * sqrt (3.) / 2., m_y - length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
	line = new gccv::Line (group, m_x - length * sqrt (3.) / 6., m_y - length / 6.,
	                       m_x - length * sqrt (3.) / 2., m_y - length / 2., NULL);
	line->SetLineColor (GO_COLOR_BLACK);
}

void gcpNewmanTool::OnRelease ()
{
	delete m_Item;
	m_Item = NULL;
	gcp::Document* doc = m_pView->GetDoc ();
	double bl = doc->GetBondLength (), dx = bl * sqrt (3.) / 2., dz = bl / 2.;
	// add the atoms and bonds
	// for now, we only add carbon atoms, and never merge with existing atoms
	// using 3d coordinates, front atoms at +bond_length/2 rear atoms at -bond_length/2
	m_x /= m_dZoomFactor;
	m_y /= m_dZoomFactor;
	gcp::Atom *atom = new gcp::Atom (6, m_x, m_y, -bl / 2.), *atom0 = new gcp::Atom (6, m_x, m_y + bl, -dz);
	doc->AddAtom (atom);
	doc->AddAtom (atom0);
	doc->AddBond (new gcp::Bond (atom, atom0, 1));
	atom0 = new gcp::Atom (6, m_x - dx, m_y - dz, -dz);
	doc->AddAtom (atom0);
	doc->AddBond (new gcp::Bond (atom, atom0, 1));
	atom0 = new gcp::Atom (6, m_x + dx, m_y - dz, -dz);
	doc->AddAtom (atom0);
	doc->AddBond (new gcp::Bond (atom, atom0, 1));
	atom0 = new gcp::Atom (6, m_x, m_y, dz);
	doc->AddAtom (atom0);
	gcp::Bond *bond = new gcp::Bond (atom, atom0, 1);
	bond->SetType (gcp::NewmanBondType);
	doc->AddBond (bond);
	atom = new gcp::Atom (6, m_x, m_y - bl, dz);
	doc->AddAtom (atom);
	doc->AddBond (new gcp::Bond (atom0, atom, 1));
	atom = new gcp::Atom (6, m_x - dx, m_y + dz, dz);
	doc->AddAtom (atom);
	doc->AddBond (new gcp::Bond (atom0, atom, 1));
	atom = new gcp::Atom (6, m_x + dx, m_y + dz, dz);
	doc->AddAtom (atom);
	doc->AddBond (new gcp::Bond (atom0, atom, 1));
	gcp::Operation *op = doc-> GetNewOperation (gcp::GCP_ADD_OPERATION);
	op->AddObject (bond->GetMolecule ());
	doc->FinishOperation ();
	// ensure that rear bonds don't go up to the center
	m_pView->Update (bond->GetMolecule ());
}

GtkWidget *gcpNewmanTool::GetPropertyPage ()
{
	gcugtk::UIBuilder *builder = new gcugtk::UIBuilder (UIDIR"/newman.ui", GETTEXT_PACKAGE);
	// FIXME: implement
	GtkWidget *res = builder->GetRefdWidget ("newman");
	delete builder;
	return res;
}
