// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * bracketstool.cc
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
#include "bracketstool.h"
#include "group.h"
#include <gcu/message.h>
#include <gcu/ui-builder.h>
#include <gcp/application.h>
#include <gcp/atom.h>
#include <gcp/bond.h>
#include <gcp/document.h>
#include <gcp/fontsel.h>
#include <gcp/fragment.h>
#include <gcp/mechanism-step.h>
#include <gcp/reaction-step.h>
#include <gcp/settings.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gccv/group.h>
#include <gccv/item-client.h>
#include <gccv/rectangle.h>
#include <glib/gi18n-lib.h>
#include <typeinfo>

gcpBracketsTool::gcpBracketsTool (gcp::Application* App): gcp::Tool (App, "Brackets")
{
	m_Type = GCP_BRACKET_NORMAL;
	m_Used = GCP_BRACKETS_BOTH;
}

gcpBracketsTool::~gcpBracketsTool ()
{
}

bool gcpBracketsTool::OnClicked ()
{
	return true;
}

void gcpBracketsTool::OnDrag ()
{
	if (m_Item) {
		reinterpret_cast <gccv::Rectangle *> (m_Item)->SetPosition (m_x0, m_y0, m_x - m_x0, m_y - m_y0);
	} else {
		m_Item = new gccv::Rectangle (m_pView->GetCanvas (), m_x0, m_y0, m_x - m_x0, m_y - m_y0);
		gcp::Theme *theme = m_pView->GetDoc ()->GetTheme ();
		static_cast <gccv::LineItem *> (m_Item)->SetLineWidth (theme->GetBondWidth ());
		static_cast <gccv::FillItem *> (m_Item)->SetFillColor (0);
	}
	// find everything inside the selected rectangle and select
	gccv::Group *group = m_pView->GetCanvas ()->GetRoot ();
	// all client top items are implemented as a child of the root
	std::list <gccv::Item *>::iterator it;
	gccv::Item *item = group->GetFirstChild (it);
	double x0, x1, y0, y1, xmin, xmax, ymin, ymax;
	m_Item->GetBounds (xmin, ymin, xmax, ymax);
	gcu::Object *object;
	m_pData->UnselectAll ();
	std::set <gcu::Object *> linked_objects;
	std::set <gcu::Object *>::iterator i, iend;
	while (item) {
		if (item != m_Item) {
			item->GetBounds (x0, y0, x1, y1);
			if ((x0 < xmax) && (y0 < ymax) && (x1 > xmin) && (y1 > ymin)) {
				object = dynamic_cast <gcu::Object *> (item->GetClient ());
				if (object && object->GetCoords (&x0, &y0) && !m_pData->IsSelected (object)) {
					x0 *= m_dZoomFactor;
					y0 *= m_dZoomFactor;
					if (x0 >= xmin && x0 <= xmax && y0 >= ymin && y0 <= ymax) {
						m_pData->SetSelected (object);
						gcp::Atom *atom = static_cast <gcp::Atom *> (object);
						switch (object->GetType ()) {
						case gcu::FragmentType:
							atom = static_cast <gcp::Fragment *> (object)->GetAtom ();
						case gcu::AtomType: {
							// go through the bonds and select them if both ends are selected
							std::map<gcu::Atom*, gcu::Bond*>::iterator i;
							gcu::Bond *bond = atom->GetFirstBond (i);
							while (bond) {
								if (m_pData->IsSelected (bond->GetAtom (atom)))
									m_pData->SetSelected (bond);
								bond = atom->GetNextBond (i);
							}
						}
						default: {
							// go through the links and store them for later treatment
							gcu::Object *linked_obj;
							linked_obj = object->GetFirstLink (i);
							while (linked_obj) {
								linked_objects.insert (linked_obj);
								linked_obj = object->GetNextLink (i);
							}
							break;
						}
						}		
					}
				}
			}
		}
		item = group->GetNextChild (it);
	}
	// now check if linked objects have all their links selected, and, if yes, select theme_change
	for (i = linked_objects.begin (), iend = linked_objects.end (); i != iend; i++)
		if ((*i)->CanSelect ())
			m_pData->SetSelected (*i);
	m_pData->SimplifySelection ();
	if (Evaluate ())
		static_cast <gccv::LineItem *> (m_Item)->SetLineColor (gcp::AddColor);
	else
		static_cast <gccv::LineItem *> (m_Item)->SetLineColor (gcp::DeleteColor);
}

void gcpBracketsTool::OnRelease ()
{
	if (Evaluate ()) {
	}
}

GtkWidget *gcpBracketsTool::GetPropertyPage ()
{
	gcu::UIBuilder *builder= NULL;
	try {
		builder = new gcu::UIBuilder (UIDIR"/brackets.ui", GETTEXT_PACKAGE);
		GtkComboBox *box = builder->GetComboBox ("type-box");
		gtk_combo_box_set_active (box, m_Type);
		g_signal_connect (box, "changed", G_CALLBACK (gcpBracketsTool::OnTypeChanged), this);
		box = builder->GetComboBox ("used-box");
		gtk_combo_box_set_active (box, m_Used);
		g_signal_connect (box, "changed", G_CALLBACK (gcpBracketsTool::OnUsedChanged), this);
		GtkBox *fbox = GTK_BOX (builder->GetWidget ("font-box"));
		GtkWidget *widget = GTK_WIDGET (g_object_new (GCP_TYPE_FONT_SEL, "allow-slanted", false, "label", "{[()]}", NULL));
		gtk_box_pack_start (fbox, widget, false, true, 0);
		gtk_widget_show_all (widget);
		m_FontSel = reinterpret_cast <GcpFontSel *> (widget);

		GtkWidget *res = builder->GetRefdWidget ("brackets");
		delete builder;
		return res;
	}
	catch (std::runtime_error &e) {
		// TODO: add a one time message box
		static bool done = false;
		if (!done) {
			done = true;
			std::string mess = _("Error loading the properties widget description: \n");
			mess += e.what ();
			new gcu::Message (GetApplication (), mess, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE);
		}
		if (builder)
			delete builder;
		return NULL;
	}
}

void gcpBracketsTool::OnTypeChanged (GtkComboBox *box, gcpBracketsTool *tool)
{
	tool->m_Type = static_cast <gcpBracketType> (gtk_combo_box_get_active (box));
}

void gcpBracketsTool::OnUsedChanged (GtkComboBox *box, gcpBracketsTool *tool)
{
	tool->m_Used = static_cast <gcpBracketsUsed> (gtk_combo_box_get_active (box));
}

void gcpBracketsTool::Activate ()
{
	gcp::Theme *theme = m_pApp->GetActiveDocument ()->GetTheme ();
	g_object_set (G_OBJECT (m_FontSel),
					"family", theme->GetTextFontFamily (),
					"weight", theme->GetTextFontWeight (),
					"variant", theme->GetTextFontVariant (),
					"stretch", theme->GetTextFontStretch (),
					"size", theme->GetTextFontSize (),
					NULL);
}

bool gcpBracketsTool::Evaluate ()
{
	gcu::Object *obj;
	if (m_pData->SelectedObjects.size () == 1) {
		obj = m_pData->SelectedObjects.front ();
		unsigned type = obj->GetType ();
		if (type == gcu::MoleculeType || type == gcp::ReactionStepType ||
		    type == gcp::MechanismStepType || type == GroupType) {
			// Evaluate bounds
			m_pData->GetObjectBounds (obj, &m_ActualBounds);
			return true;
		}
	} else {
	}
	return false;
}
